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
#include "Lamp/Rendering/Swapchain.h"
#include "Lamp/Rendering/Cameras/PerspectiveCamera.h"
#include "Lamp/Rendering/RendererNew.h"

#include "Lamp/Level/Level.h"
#include "Lamp/Mesh/Materials/MaterialLibrary.h"

#include <random>

//TODO: remove
#include "Lamp/AssetSystem/MeshImporter.h"

namespace Lamp
{
	Renderer::SceneData* Renderer::s_pSceneData = nullptr;
	Ref<RendererNew> Renderer::s_renderer = nullptr;
	Scope<Renderer::RendererDefaults> Renderer::s_rendererDefaults;

	Renderer::Capabilities Renderer::s_capabilities;
	Renderer::Statistics Renderer::s_statistics;

	RenderBuffer Renderer::s_firstRenderBuffer;
	RenderBuffer Renderer::s_secondRenderBuffer;

	RenderBuffer* Renderer::s_submitBufferPointer = &s_firstRenderBuffer;
	RenderBuffer* Renderer::s_renderBufferPointer = &s_secondRenderBuffer;

	static const std::filesystem::path s_defaultTexturePath = "engine/textures/default/defaultTexture.png";

	void Renderer::Initialize()
	{
		LP_PROFILE_FUNCTION();

		s_renderer = RendererNew::Create();
		s_renderer->Initialize();

		s_pSceneData = new Renderer::SceneData();

		s_rendererDefaults = CreateScope<RendererDefaults>();
		s_rendererDefaults->defaultTexture = Texture2D::Create(s_defaultTexturePath);

		s_firstRenderBuffer.drawCalls.reserve(500);
		s_secondRenderBuffer.drawCalls.reserve(500);

		ShaderLibrary::AddShader("engine/shaders/vulkan/vulkanPbr.glsl"); //TODO: remove
		ShaderLibrary::AddShader("engine/shaders/vulkan/vulkanQuad.glsl");
		ShaderLibrary::AddShader("engine/shaders/vulkan/vulkanDepthPrePass.glsl");
		ShaderLibrary::AddShader("engine/shaders/vulkan/vulkanSSAO.glsl");
		MaterialLibrary::LoadMaterials();

		GenerateKernel();
		s_pSceneData->ssaoNoiseTexture = Texture2D::Create(4, 4);
		s_pSceneData->ssaoNoiseTexture->SetData(s_pSceneData->ssaoNoise.data(), s_pSceneData->ssaoNoise.size() * sizeof(glm::vec4));

		SetupBuffers();
	}

	void Renderer::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		delete s_pSceneData;
	}

	void Renderer::Begin(const Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();

		UpdateBuffers(camera);

		s_renderer->Begin(camera);
	}

	void Renderer::End()
	{
		s_statistics.totalDrawCalls = 0;
		s_statistics.memoryStatistics = s_renderer->GetMemoryUsage();

		s_renderer->End();
		s_submitBufferPointer->drawCalls.clear();
	}

	void Renderer::BeginPass(const Ref<RenderPipeline> pipeline)
	{
		UpdatePassBuffers(pipeline);
		s_renderer->BeginPass(pipeline);
	}

	void Renderer::EndPass()
	{
		s_renderer->EndPass();
	}

	void Renderer::SwapBuffers()
	{
		std::swap(s_submitBufferPointer, s_renderBufferPointer);
		s_submitBufferPointer->drawCalls.clear();
	}

	void Renderer::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id)
	{
		s_submitBufferPointer->drawCalls.emplace_back(transform, mesh, material, id);
	}

	void Renderer::DrawBuffer()
	{
		s_renderer->DrawBuffer(*s_submitBufferPointer);
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
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		//Set data in uniform buffers
		
		//Camera data
		{
			auto ub = s_pSceneData->uniformBufferSet->Get(0, 0, currentFrame);

			Ref<PerspectiveCamera> perspectiveCamera = std::dynamic_pointer_cast<PerspectiveCamera>(camera);
			float tanHalfFOV = glm::tan(glm::radians(perspectiveCamera->GetFieldOfView()) / 2.f);

			s_pSceneData->cameraData.positionAndTanHalfFOV = glm::vec4(camera->GetPosition(), tanHalfFOV);
			s_pSceneData->cameraData.projection = camera->GetProjectionMatrix();
			s_pSceneData->cameraData.view = camera->GetViewMatrix();

			ub->SetData(&s_pSceneData->cameraData, sizeof(CameraDataBuffer));
		}

		//Directional lights
		{
			auto ub = s_pSceneData->uniformBufferSet->Get(1, 0, currentFrame);

			uint32_t index = 0;
			s_pSceneData->directionalLightDataBuffer.lightCount = 0;
			for (const auto& light : g_pEnv->pLevel->GetRenderUtils().GetDirectionalLights())
			{
				glm::vec3 direction = glm::normalize(glm::mat3(light->transform) * glm::vec3(1.f));

				s_pSceneData->directionalLightDataBuffer.dirLights[index].direction = glm::vec4(direction, 1.f);
				s_pSceneData->directionalLightDataBuffer.dirLights[index].colorIntensity = glm::vec4(light->color, light->intensity);
				s_pSceneData->directionalLightDataBuffer.dirLights[index].castShadows = light->castShadows;
				s_pSceneData->directionalLightDataBuffer.lightCount++;

				index++;
				if (index > 0)
				{
					break;
				}
			}

			ub->SetData(&s_pSceneData->directionalLightDataBuffer, sizeof(DirectionalLightDataBuffer));
		}

		//SSAO
		{
			auto ub = s_pSceneData->uniformBufferSet->Get(2, 0, currentFrame);
			ub->SetData(&s_pSceneData->ssaoData, sizeof(SSAODataBuffer));
		}

	#if 0

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

		s_pSceneData->screenGroupX = ((uint32_t)s_pSceneData->bufferSize.x + ((uint32_t)s_pSceneData->bufferSize.x % s_pSceneData->screenTileSize)) / s_pSceneData->screenTileSize;
		s_pSceneData->screenGroupY = ((uint32_t)s_pSceneData->bufferSize.y + ((uint32_t)s_pSceneData->bufferSize.y % s_pSceneData->screenTileSize)) / s_pSceneData->screenTileSize;
		s_pSceneData->screenTileCount = s_pSceneData->screenGroupX * s_pSceneData->screenGroupY;


	
	#endif

	}

	void Renderer::UpdatePassBuffers(const Ref<RenderPipeline> pipeline)
	{
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		//Screen
		{
			auto ub = s_pSceneData->uniformBufferSet->Get(3, 0, currentFrame);

			s_pSceneData->screenData.size = { (float)pipeline->GetSpecification().framebuffer->GetSpecification().width, (float)pipeline->GetSpecification().framebuffer->GetSpecification().height };
			s_pSceneData->screenData.aspectRatio = s_pSceneData->screenData.size.x / s_pSceneData->screenData.size.y;

			ub->SetData(&s_pSceneData->screenData, sizeof(ScreenDataBuffer));
		}
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
			s_pSceneData->ssaoNoise.push_back(glm::vec4(noise, 0.f));
		}
	}

	void Renderer::SetupBuffers()
	{
		s_pSceneData->uniformBufferSet = UniformBufferSet::Create(Renderer::GetCapabilities().framesInFlight);
		s_pSceneData->uniformBufferSet->Add(&s_pSceneData->cameraData, sizeof(CameraDataBuffer), 0, 0);
		s_pSceneData->uniformBufferSet->Add(&s_pSceneData->directionalLightDataBuffer, sizeof(DirectionalLightDataBuffer), 1, 0);
		s_pSceneData->uniformBufferSet->Add(&s_pSceneData->ssaoData, sizeof(SSAODataBuffer), 2, 0);
		s_pSceneData->uniformBufferSet->Add(&s_pSceneData->screenData, sizeof(ScreenDataBuffer), 3, 0);
	}
}