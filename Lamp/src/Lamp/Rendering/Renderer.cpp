#include "lppch.h"
#include "Renderer.h"

#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"
#include "Lamp/Rendering/Buffers/ShaderStorageBuffer.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/CommandBuffer.h"
#include "Lamp/Rendering/RenderCommand.h"
#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Renderer3D.h"

#include "Lamp/Rendering/RendererNew.h"

#include "Lamp/Level/Level.h"
#include "Lamp/Mesh/Materials/MaterialLibrary.h"

#include <random>

//TODO: remove
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include <vulkan/vulkan_core.h>
#include "Lamp/AssetSystem/MeshImporter.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanRenderPipeline.h"

namespace Lamp
{
	Renderer::SceneData* Renderer::s_pSceneData = nullptr;
	Ref<RendererNew> Renderer::s_renderer = nullptr;
	Renderer::Capabilities Renderer::s_capabilities;
	Scope<Renderer::RendererDefaults> Renderer::s_rendererDefaults;

	static const std::filesystem::path s_defaultTexturePath = "engine/textures/default/defaultTexture.png";

	void Renderer::Initialize()
	{
		LP_PROFILE_FUNCTION();
		s_rendererDefaults = CreateScope<RendererDefaults>();
		s_rendererDefaults->defaultTexture = Texture2D::Create(s_defaultTexturePath);

		s_renderer = RendererNew::Create();
		s_renderer->Initialize();

		s_pSceneData = new Renderer::SceneData();
	}

	void Renderer::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		delete s_pSceneData;
		//Renderer3D::Shutdown();
	}

	void Renderer::Begin(const Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();

		//g_pEnv->pLevel->SetSkybox("assets/textures/frozen_waterfall.hdr");

		//UpdateBuffers(camera);
		//Renderer3D::Begin(camera);
	
		s_renderer->Begin(camera);
	}

	void Renderer::End()
	{
		//Renderer3D::End();
		s_renderer->End();
	}

	void Renderer::CreateUniformBuffers()
	{
		GenerateKernel();
	}

	void Renderer::CreateShaderStorageBuffers()
	{
		s_pSceneData->pointLightStorageBuffer = ShaderStorageBuffer::Create(s_pSceneData->maxLights * sizeof(PointLightData), 2);
		
		s_pSceneData->screenGroupX = (s_pSceneData->maxScreenTileBufferAlloc + (s_pSceneData->maxScreenTileBufferAlloc % s_pSceneData->screenTileSize)) / s_pSceneData->screenTileSize;
		s_pSceneData->screenGroupY = (s_pSceneData->maxScreenTileBufferAlloc + (s_pSceneData->maxScreenTileBufferAlloc % s_pSceneData->screenTileSize)) / s_pSceneData->screenTileSize;
		s_pSceneData->screenTileCount = s_pSceneData->screenGroupX * s_pSceneData->screenGroupY;
		s_pSceneData->visibleLightsStorageBuffer = ShaderStorageBuffer::Create(s_pSceneData->screenTileCount * sizeof(LightIndex) * s_pSceneData->maxLights, 3, DrawAccess::Static);
	}

	void Renderer::UpdateBuffers(const Ref<CameraBase> camera)
	{
		//Set data in uniform buffers
		//Common data
		{
			s_pSceneData->commonRenderData.cameraPosition = glm::vec4(camera->GetPosition(), 0.f);
			s_pSceneData->commonRenderData.projection = camera->GetProjectionMatrix();
			s_pSceneData->commonRenderData.view = camera->GetViewMatrix();
			s_pSceneData->commonDataBuffer->SetData(&s_pSceneData->commonRenderData, sizeof(CommonRenderData));
		}

		//Directional lights
		{
			uint32_t index = 0;
			s_pSceneData->directionalLightData.lightCount = 0;
			for (const auto& light : g_pEnv->pLevel->GetRenderUtils().GetDirectionalLights())
			{
				glm::vec3 direction = glm::normalize(glm::mat3(light->transform) * glm::vec3(1.f));

				s_pSceneData->directionalLightData.dirLights[index].direction = glm::vec4(direction, 1.f);
				s_pSceneData->directionalLightData.dirLights[index].colorIntensity = glm::vec4(light->color, light->intensity);
				s_pSceneData->directionalLightData.dirLights[index].castShadows = light->castShadows;
				s_pSceneData->directionalLightData.lightCount++;

				index++;
			}

			s_pSceneData->directionalLightBuffer->SetData(&s_pSceneData->directionalLightData, sizeof(DirectionalLightBuffer));
		}

		//Light data
		{
			uint32_t index = 0;
			for (const auto& light : g_pEnv->pLevel->GetRenderUtils().GetDirectionalLights())
			{
				s_pSceneData->directionalLightVPData.viewProjections[index] = light->viewProjection;
				index++;
			}
			s_pSceneData->directionalLightVPData.lightCount = index;
			s_pSceneData->directionalLightVPBuffer->SetData(&s_pSceneData->directionalLightVPData, sizeof(DirectionalLightVPs));
		}

		//SSAO
		{
			s_pSceneData->ssaoBuffer->SetData(&s_pSceneData->ssaoData, sizeof(SSAOData));
		}

		s_pSceneData->screenGroupX = ((uint32_t)s_pSceneData->bufferSize.x + ((uint32_t)s_pSceneData->bufferSize.x % s_pSceneData->screenTileSize)) / s_pSceneData->screenTileSize;
		s_pSceneData->screenGroupY = ((uint32_t)s_pSceneData->bufferSize.y + ((uint32_t)s_pSceneData->bufferSize.y % s_pSceneData->screenTileSize)) / s_pSceneData->screenTileSize;
		s_pSceneData->screenTileCount = s_pSceneData->screenGroupX * s_pSceneData->screenGroupY;

		//SSAO
		Ref<PerspectiveCamera> perspectiveCamera = std::dynamic_pointer_cast<PerspectiveCamera>(camera);
		s_pSceneData->aspectRatio = perspectiveCamera->GetAspectRatio();
		s_pSceneData->tanHalfFOV = glm::tan(glm::radians(s_pSceneData->aspectRatio) / 2.f);
	}

	static float Lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void Renderer::GenerateKernel()
	{
		s_pSceneData->ssaoNoise.clear();

		std::uniform_real_distribution<float> randomFloats(0.f, 1.f);
		std::default_random_engine generator;

		for (uint32_t i = 0; i < s_pSceneData->ssaoData.kernelSize; i++)
		{
			glm::vec3 sample{ randomFloats(generator) * 2.f - 1.f, randomFloats(generator) * 2.f - 1.f, randomFloats(generator) };
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			float scale = float(i) / s_pSceneData->ssaoData.kernelSize;

			scale = Lerp(0.1f, 1.f, scale * scale);
			sample *= scale;

			s_pSceneData->ssaoData.kernelSamples[i] = glm::vec4(sample, 0.f);
		}

		for (uint32_t i = 0; i < 16; i++)
		{
			glm::vec3 noise{ randomFloats(generator) * 2.f - 1.f, randomFloats(generator) * 2.f - 1.f, 0.f };
			s_pSceneData->ssaoNoise.push_back(noise);
		}
	}

	void Renderer::Draw()
	{
		s_renderer->SubmitMesh(glm::mat4(1.f), nullptr, nullptr);
	}
}