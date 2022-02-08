#include "lppch.h"
#include "Renderer.h"

#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"
#include "Lamp/Rendering/Buffers/ShaderStorageBuffer.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/Textures/TextureCube.h"
#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/CommandBuffer.h"
#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Swapchain.h"
#include "Lamp/Rendering/Cameras/PerspectiveCamera.h"

#include "Lamp/Level/Level.h"
#include "Lamp/Mesh/Materials/MaterialLibrary.h"
#include "Lamp/Core/Time/ScopedTimer.h"

#include "Platform/Vulkan/VulkanRenderer.h"

#include <random>

//TODO: remove
#include "Lamp/AssetSystem/MeshImporter.h"

namespace Lamp
{
	Renderer::SceneData* Renderer::s_pSceneData = nullptr;
	Ref<VulkanRenderer> Renderer::s_renderer = nullptr;
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

		s_renderer = VulkanRenderer::Create();
		s_renderer->Initialize();

		s_pSceneData = new Renderer::SceneData();

		s_rendererDefaults = CreateScope<RendererDefaults>();
		s_rendererDefaults->defaultTexture = Texture2D::Create(s_defaultTexturePath);

		s_firstRenderBuffer.drawCalls.reserve(500);
		s_secondRenderBuffer.drawCalls.reserve(500);

		ShaderLibrary::LoadShaders();
		MaterialLibrary::LoadMaterials();

		CreateUniformBuffers();
		GenerateBRDF();

		uint32_t blackCubeTextureData[6] = { 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000 };
		s_pSceneData->blackCubeTexture = TextureCube::Create(ImageFormat::RGBA, 1, 1, &blackCubeTextureData);

		uint32_t whiteTextureData = 0xffffffff;
		s_pSceneData->whiteTexture = Texture2D::Create(ImageFormat::RGBA, 1, 1);
		s_pSceneData->whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
	}

	void Renderer::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		delete s_pSceneData;
	}

	void Renderer::Begin(const Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();

		s_statistics.memoryStatistics = s_renderer->GetMemoryUsage();
		s_pSceneData->camera = camera;

		DrawDirectionalShadows();

		UpdateBuffers(camera);
		SortRenderBuffer(camera->GetPosition(), *s_renderBufferPointer);
		s_renderer->Begin(camera);
	}

	void Renderer::End()
	{
		LP_PROFILE_FUNCTION();
		s_renderer->End();
	}

	void Renderer::BeginPass(const Ref<RenderPipeline> pipeline)
	{
		LP_PROFILE_FUNCTION();
		UpdatePassBuffers(pipeline);
		s_renderer->BeginPass(pipeline);
	}

	void Renderer::EndPass()
	{
		LP_PROFILE_FUNCTION();
		s_renderer->EndPass();
	}

	void Renderer::SwapBuffers()
	{
		std::swap(s_submitBufferPointer, s_renderBufferPointer);
		s_submitBufferPointer->drawCalls.clear();
		s_statistics.totalDrawCalls = 0;
	}

	void Renderer::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id)
	{
		s_submitBufferPointer->drawCalls.emplace_back(transform, mesh, material, id);
		s_statistics.totalDrawCalls++;
	}

	void Renderer::SubmitMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant)
	{
		s_renderer->SubmitMesh(mesh, material, descriptorSets, pushConstant);
	}

	void Renderer::SubmitQuad()
	{
		s_renderer->SubmitQuad();
	}

	void Renderer::DrawBuffer()
	{
		LP_PROFILE_FUNCTION();
		s_renderer->DrawBuffer(*s_renderBufferPointer);
	}

	void Renderer::UpdateBuffers(const Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		//Set data in uniform buffers

		//Camera data
		{
			auto ub = s_pSceneData->uniformBufferSet->Get(0, 0, currentFrame);

			Ref<PerspectiveCamera> perspectiveCamera = std::dynamic_pointer_cast<PerspectiveCamera>(camera);
			float tanHalfFOV = glm::tan(glm::radians(perspectiveCamera->GetFieldOfView()) / 2.f);

			s_pSceneData->cameraData.ambienceExposure.x = s_pSceneData->ambianceMultiplier;
			s_pSceneData->cameraData.ambienceExposure.y = s_pSceneData->hdrExposure;
			s_pSceneData->cameraData.positionAndTanHalfFOV = glm::vec4(camera->GetPosition(), tanHalfFOV);
			s_pSceneData->cameraData.projection = camera->GetProjectionMatrix();
			s_pSceneData->cameraData.view = camera->GetViewMatrix();

			ub->SetData(&s_pSceneData->cameraData, sizeof(CameraDataBuffer));
		}

		//Directional lights
		if (LevelManager::IsLevelLoaded())
		{
			auto ub = s_pSceneData->uniformBufferSet->Get(1, 0, currentFrame);

			uint32_t index = 0;
			s_pSceneData->directionalLightDataBuffer.lightCount = 0;
			for (const auto& light : LevelManager::GetActive()->GetEnvironment().GetDirectionalLights())
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

		//Terrain data
		{
			s_pSceneData->terrainDataBuffer->SetData(&s_pSceneData->terrainData, sizeof(TerrainDataBuffer));
		}


		//Light data
		if (LevelManager::IsLevelLoaded())
		{
			uint32_t index = 0;
			for (const auto& light : LevelManager::GetActive()->GetEnvironment().GetDirectionalLights())
			{
				s_pSceneData->directionalLightVPData.directionalLightVPs[index] = light->viewProjection;
				index++;
			}
			s_pSceneData->directionalLightVPData.count = index;

			auto ub = s_pSceneData->uniformBufferSet->Get(4, 0, currentFrame);
			ub->SetData(&s_pSceneData->directionalLightVPData, sizeof(DirectionalLightVPBuffer));
		}

		//Point lights
		if (LevelManager::IsLevelLoaded())
		{
			auto& pointLights = LevelManager::GetActive()->GetEnvironment().GetPointLights();
			auto pointlightStorageBuffer = s_pSceneData->shaderStorageBufferSet->Get(12, 0, currentFrame);

			PointLightData* buffer = (PointLightData*)pointlightStorageBuffer->Map();

			s_pSceneData->lightCullingData.lightCount = 0;

			for (uint32_t i = 0; i < pointLights.size(); i++)
			{
				const auto& light = pointLights[i];

				buffer[i].position = glm::vec4(light->shadowBuffer->GetPosition(), 1.f);
				buffer[i].color = glm::vec4(light->color, 0.f);
				buffer[i].intensity = light->intensity;
				buffer[i].falloff = light->falloff;
				buffer[i].farPlane = light->farPlane;
				buffer[i].radius = light->radius;
				buffer[i].samplerId = i;

				s_pSceneData->lightCullingData.lightCount++;
			}

			pointlightStorageBuffer->Unmap();

			s_pSceneData->lightCullingBuffer->SetData(&s_pSceneData->lightCullingData, sizeof(LightCullingBuffer));
		}
	}

	void Renderer::UpdatePassBuffers(const Ref<RenderPipeline> pipeline)
	{
		LP_PROFILE_FUNCTION();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		//Screen
		{
			auto ub = s_pSceneData->uniformBufferSet->Get(3, 0, currentFrame);

			s_pSceneData->screenData.size = { (float)pipeline->GetSpecification().framebuffer->GetSpecification().width, (float)pipeline->GetSpecification().framebuffer->GetSpecification().height };
			s_pSceneData->screenData.aspectRatio = s_pSceneData->screenData.size.x / s_pSceneData->screenData.size.y;
			s_pSceneData->screenData.xScreenTiles = s_pSceneData->screenGroupX;

			ub->SetData(&s_pSceneData->screenData, sizeof(ScreenDataBuffer));
		}

		//Light culling buffer
		{
			glm::vec2 size = { (float)pipeline->GetSpecification().framebuffer->GetSpecification().width, (float)pipeline->GetSpecification().framebuffer->GetSpecification().height };
			
			s_pSceneData->screenGroupX = ((uint32_t)size.x + (16 - ((uint32_t)size.x % 16))) / 16;
			s_pSceneData->screenGroupY = ((uint32_t)size.y + (16 - ((uint32_t)size.y % 16))) / 16;

			s_pSceneData->screenTileCount = s_pSceneData->screenGroupX * s_pSceneData->screenGroupY;
			
			s_pSceneData->lightCullingData.screenSize = size;
			s_pSceneData->lightCullingBuffer->SetData(&s_pSceneData->lightCullingData.screenSize, sizeof(glm::vec2));
		}

		//HBAO
		{
			auto& hbaoData = s_pSceneData->hbaoData;

			const float meters2ViewSpace = 1.f;
			const float R = s_pSceneData->hbaoRadius * meters2ViewSpace;
			const float R2 = R * R;

			hbaoData.negInvR2 = -1.f / R2;
			
			glm::vec2 size = { (float)pipeline->GetSpecification().framebuffer->GetSpecification().width, (float)pipeline->GetSpecification().framebuffer->GetSpecification().height };
			hbaoData.invQuarterResolution = 1.f / glm::vec2{ size.x / 4.f, size.y / 4.f };

			if (s_pSceneData->camera)
			{
				hbaoData.radiusToScreen = R * 0.5f * size.y / (tanf(glm::radians(std::reinterpret_pointer_cast<PerspectiveCamera>(s_pSceneData->camera)->GetFieldOfView()) * 0.5f) * 2.f);
	
				const float* P = glm::value_ptr(s_pSceneData->camera->GetProjectionMatrix());
				const glm::vec4 projInfoPerspective =
				{
					2.f / (P[4 * 0 + 0]),
					2.f / (P[4 * 1 + 1]),
					-(1.f - P[4 * 2 + 0]) / P[4 * 0 + 0],
					-(1.f + P[4 * 2 + 1]) / P[4 * 1 + 1]
				};

				hbaoData.perspectiveInfo = projInfoPerspective;
			}
	
			hbaoData.isOrtho = false;
			hbaoData.powExponent = glm::max(s_pSceneData->hbaoIntensity, 0.f);
			hbaoData.NdotVBias = glm::min(glm::max(0.f, s_pSceneData->hbaoBias), 1.f);
			hbaoData.aoMultiplier = 1.f / (1.f - hbaoData.NdotVBias);
			
			s_pSceneData->hbaoDataBuffer->SetData(&s_pSceneData->hbaoData, sizeof(HBAODataBuffer));
		}
	}

	void Renderer::DrawDirectionalShadows()
	{
		LP_PROFILE_FUNCTION();

		if (!LevelManager::IsLevelLoaded())
		{
			return;
		}

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		for (const auto& light : LevelManager::GetActive()->GetEnvironment().GetDirectionalLights())
		{
			if (!light->castShadows)
			{
				continue;
			}

			auto ub = light->shadowPipeline->GetSpecification().uniformBufferSets->Get(0, 0, currentFrame);
			ub->SetData(&light->viewProjection, sizeof(glm::mat4));

			BeginPass(light->shadowPipeline);

			DrawBuffer();

			EndPass();
		}
	}

	void Renderer::GenerateBRDF()
	{
		ScopedTimer timer{ "Generate BRDFLUT" };

		const uint32_t brdfDim = 512;

		FramebufferSpecification framebufferSpec{};
		framebufferSpec.swapchainTarget = false;
		framebufferSpec.width = brdfDim;
		framebufferSpec.height = brdfDim;
		framebufferSpec.attachments =
		{ 
			ImageFormat::RG16F
		};

		RenderPipelineSpecification pipelineSpec{};
		pipelineSpec.framebuffer = Framebuffer::Create(framebufferSpec);
		s_pSceneData->brdfFramebuffer = pipelineSpec.framebuffer;

		pipelineSpec.shader = ShaderLibrary::GetShader("BRDFIntegrate");
		pipelineSpec.isSwapchain = false;
		pipelineSpec.cullMode = CullMode::Front;
		pipelineSpec.topology = Topology::TriangleList;
		pipelineSpec.drawType = DrawType::Quad;
		pipelineSpec.uniformBufferSets = Renderer::GetSceneData()->uniformBufferSet;
		pipelineSpec.vertexLayout =
		{
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float3, "a_Tangent" },
			{ ElementType::Float3, "a_Bitangent" },
			{ ElementType::Float2, "a_TexCoords" }
		};

		auto renderPass = RenderPipeline::Create(pipelineSpec);

		Renderer::BeginPass(renderPass);
		Renderer::SubmitQuad();
		Renderer::EndPass();
	}

	void Renderer::SortRenderBuffer(const glm::vec3& sortPoint, RenderBuffer& buffer)
	{
		std::sort(buffer.drawCalls.begin(), buffer.drawCalls.end(), [&sortPoint](const RenderCommandData& dataOne, const RenderCommandData& dataTwo)
			{
				const glm::vec3& dPosOne = dataOne.transform[3];
				const glm::vec3& dPosTwo = dataTwo.transform[3];

				const float distTwo = glm::pow(sortPoint.x - dPosTwo.x, 2.f) + glm::pow(sortPoint.y - dPosTwo.y, 2.f) + glm::pow(sortPoint.z - dPosTwo.z, 2.f);
				const float distOne = glm::pow(sortPoint.x - dPosOne.x, 2.f) + glm::pow(sortPoint.y - dPosOne.y, 2.f) + glm::pow(sortPoint.z - dPosOne.z, 2.f);

				return distOne < distTwo;
			});
	}

	static float Lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	std::pair<Ref<RenderComputePipeline>, std::function<void()>> Renderer::CreateLightCullingPipeline(Ref<Image2D> depthImage)
	{
		return s_renderer->CreateLightCullingPipeline(s_pSceneData->uniformBufferSet->Get(0), s_pSceneData->lightCullingBuffer, s_pSceneData->shaderStorageBufferSet, depthImage);
	}

	std::pair<Ref<RenderComputePipeline>, std::function<void()>> Renderer::CreateHBAOPipeline(Ref<Image2D> depthImage)
	{
		return std::pair<Ref<RenderComputePipeline>, std::function<void()>>();
	}

	void Renderer::CreateUniformBuffers()
	{
		/////Uniform buffer//////
		s_pSceneData->uniformBufferSet = UniformBufferSet::Create(Renderer::GetCapabilities().framesInFlight);
		s_pSceneData->uniformBufferSet->Add(&s_pSceneData->cameraData, sizeof(CameraDataBuffer), 0, 0);
		s_pSceneData->uniformBufferSet->Add(&s_pSceneData->directionalLightDataBuffer, sizeof(DirectionalLightDataBuffer), 1, 0);
		s_pSceneData->uniformBufferSet->Add(&s_pSceneData->screenData, sizeof(ScreenDataBuffer), 3, 0);
		s_pSceneData->uniformBufferSet->Add(&s_pSceneData->directionalLightVPData, sizeof(DirectionalLightVPBuffer), 4, 0);

		s_pSceneData->lightCullingBuffer = UniformBuffer::Create(&s_pSceneData->lightCullingData, sizeof(LightCullingBuffer));
		s_pSceneData->hbaoDataBuffer = UniformBuffer::Create(&s_pSceneData->hbaoData, sizeof(HBAODataBuffer));
		s_pSceneData->terrainDataBuffer = UniformBuffer::Create(&s_pSceneData->terrainData, sizeof(TerrainDataBuffer));

		/////Shader storage/////
		s_pSceneData->screenGroupX = (s_pSceneData->maxScreenTileBufferAlloc + (s_pSceneData->maxScreenTileBufferAlloc % s_pSceneData->screenTileSize)) / s_pSceneData->screenTileSize;
		s_pSceneData->screenGroupY = (s_pSceneData->maxScreenTileBufferAlloc + (s_pSceneData->maxScreenTileBufferAlloc % s_pSceneData->screenTileSize)) / s_pSceneData->screenTileSize;
		s_pSceneData->screenTileCount = s_pSceneData->screenGroupX * s_pSceneData->screenGroupY;

		s_pSceneData->shaderStorageBufferSet = ShaderStorageBufferSet::Create(Renderer::GetCapabilities().framesInFlight);
		s_pSceneData->shaderStorageBufferSet->Add(s_pSceneData->maxLights * sizeof(PointLightData), 12, 0);
		s_pSceneData->shaderStorageBufferSet->Add(s_pSceneData->screenTileCount * sizeof(LightIndex) * s_pSceneData->maxLights, 13, 0);
	}
}