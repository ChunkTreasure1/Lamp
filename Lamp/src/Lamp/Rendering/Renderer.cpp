#include "lppch.h"
#include "Renderer.h"

#include "RenderGraph/Nodes/DynamicUniformRegistry.h"
#include "Shadows/PointShadowBuffer.h"
#include "Buffers/ShaderStorageBuffer.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "Renderer3D.h"

#include "Lamp/Level/Level.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Mesh/Materials/MaterialLibrary.h"

#include <random>

namespace Lamp
{
	Renderer::SceneData* Renderer::s_pSceneData = new Renderer::SceneData;

	void Renderer::Initialize()
	{
		LP_PROFILE_FUNCTION();
		ShaderLibrary::LoadShaders();
		MaterialLibrary::LoadMaterials();

		RenderCommand::Initialize();
		Renderer3D::Initialize();
		Renderer2D::Initialize();

		CreateUniformBuffers();
		CreateShaderStorageBuffers();

		s_pSceneData->ssaoNoiseTexture = Texture2D::Create(4, 4);
		s_pSceneData->ssaoNoiseTexture->SetData(s_pSceneData->ssaoNoise.data(), 0);
		s_pSceneData->internalTextures.emplace("SSAO Noise", s_pSceneData->ssaoNoiseTexture);

		//Setup dynamic uniforms
		DynamicUniformRegistry::AddUniform("Exposure", UniformType::Float, RegisterData(&s_pSceneData->hdrExposure));
		DynamicUniformRegistry::AddUniform("Gamma", UniformType::Float, RegisterData(&s_pSceneData->gamma));
		DynamicUniformRegistry::AddUniform("Buffer Size", UniformType::Float2, RegisterData(&s_pSceneData->bufferSize));
		DynamicUniformRegistry::AddUniform("ForwardTileX", UniformType::Int, RegisterData(&s_pSceneData->screenGroupX));
		DynamicUniformRegistry::AddUniform("SSAO kernel size", UniformType::Int, RegisterData(&s_pSceneData->ssaoKernelSize));
		DynamicUniformRegistry::AddUniform("Aspect ratio", UniformType::Float, RegisterData(&s_pSceneData->aspectRatio));
		DynamicUniformRegistry::AddUniform("Tan Half FOV", UniformType::Float, RegisterData(&s_pSceneData->tanHalfFOV));
	}

	void Renderer::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		Renderer3D::Shutdown();
	}

	void Renderer::Begin(const Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();

		UpdateBuffers(camera);
		Renderer3D::Begin(camera);
	}

	void Renderer::End()
	{
		Renderer3D::End();
	}

	void Renderer::CreateUniformBuffers()
	{
		s_pSceneData->commonDataBuffer = UniformBuffer::Create(sizeof(CommonRenderData), 0);
		s_pSceneData->directionalLightBuffer = UniformBuffer::Create(sizeof(DirectionalLightBuffer), 1);
		s_pSceneData->directionalLightVPBuffer = UniformBuffer::Create(sizeof(DirectionalLightVPs), 4);
		
		s_pSceneData->ssaoBuffer = UniformBuffer::Create(sizeof(SSAOData), 6);
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
				s_pSceneData->directionalLightData.lightCount++;

				index++;
			}

			s_pSceneData->directionalLightBuffer->SetData(&s_pSceneData->directionalLightData, sizeof(DirectionalLightBuffer));
		}

		//Point lights
		{
			PointLightData* buffer = (PointLightData*)s_pSceneData->pointLightStorageBuffer->Map();
			s_pSceneData->pointLightCount = 0;

			for (uint32_t i = 0; i < g_pEnv->pLevel->GetRenderUtils().GetPointLights().size(); i++)
			{
				const auto& light = g_pEnv->pLevel->GetRenderUtils().GetPointLights()[i];

				buffer[i].position = glm::vec4(light->shadowBuffer->GetPosition(), 0.f); //TODO: Change to using other position
				buffer[i].color = glm::vec4(light->color, 0.f);
				buffer[i].intensity = light->intensity;
				buffer[i].falloff = light->falloff;
				buffer[i].farPlane = light->farPlane;
				buffer[i].radius = light->radius;

				s_pSceneData->pointLightCount++;
			}

			s_pSceneData->pointLightStorageBuffer->Unmap();
		}

		//Light data
		{
			uint32_t index = 0;
			for (const auto& light : g_pEnv->pLevel->GetRenderUtils().GetDirectionalLights())
			{
				if (!light->castShadows)
				{
					continue;
				}

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
}