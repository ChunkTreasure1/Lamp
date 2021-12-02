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
	struct TempRendererStorage
	{
		Ref<RenderPipeline> mainPipeline;
		Ref<Shader> mainShader;
		Ref<CommandBuffer> commandBuffer;

		TestUniformBuffer uniformBuffer;
		Ref<UniformBufferSet> uniformBufferSet;

		Ref<Mesh> teddy;
		Ref<Texture2D> teddyTexture;
	};

	static TempRendererStorage* s_pTempStorage = nullptr;
	static VkDescriptorPool s_descriptorPool; //TODO: should renderer be abstracted?


	Renderer::SceneData* Renderer::s_pSceneData = nullptr;
	Renderer::Capabilities Renderer::s_capabilities;


	void Renderer::Initialize()
	{
		LP_PROFILE_FUNCTION();
		s_pSceneData = new Renderer::SceneData();
		s_pTempStorage = new TempRendererStorage();

		//TODO: this should be moved into wrapper
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 10000;
		poolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		auto device = VulkanContext::GetCurrentDevice();
		VkResult result = vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &s_descriptorPool);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create descriptor pool!");

		SetupBuffers();
		s_pTempStorage->mainShader = Shader::Create("engine/shaders/vulkan/testShader.glsl", false);

		RenderPipelineSpecification pipelineSpec{};
		pipelineSpec.shader = s_pTempStorage->mainShader;
		pipelineSpec.isSwapchain = true;
		pipelineSpec.topology = Topology::TriangleList;
		pipelineSpec.uniformBufferSets = s_pTempStorage->uniformBufferSet;
		pipelineSpec.vertexLayout =
		{
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float3, "a_Tangent" },
			{ ElementType::Float3, "a_Bitangent" },
			{ ElementType::Float2, "a_TexCoords" },
		};

		s_pTempStorage->mainPipeline = RenderPipeline::Create(pipelineSpec);

		MeshImportSettings settings;
		settings.path = "assets/meshes/teddy/teddy.fbx";
		s_pTempStorage->teddy = MeshImporter::ImportMesh(settings);
		s_pTempStorage->teddyTexture = Texture2D::Create("assets/textures/TeddyTextures/DJTeddy_final_albedo.tga");

		s_pTempStorage->commandBuffer = CommandBuffer::Create(s_pTempStorage->mainPipeline);
	}

	void Renderer::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		delete s_pTempStorage;
		delete s_pSceneData;
		//Renderer3D::Shutdown();
	}

	void Renderer::Begin(const Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();

		//g_pEnv->pLevel->SetSkybox("assets/textures/frozen_waterfall.hdr");

		//UpdateBuffers(camera);
		//Renderer3D::Begin(camera);
	
	
		
		s_pTempStorage->commandBuffer->Begin();

		auto swapchain = std::dynamic_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());
		const uint32_t currentFrame = swapchain->GetCurrentFrame();

		VkRenderPassBeginInfo renderPassBegin{};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.renderPass = swapchain->GetRenderPass();
		renderPassBegin.framebuffer = swapchain->GetFramebuffer(currentFrame);
		renderPassBegin.renderArea.offset = { 0, 0 };
		renderPassBegin.renderArea.extent = swapchain->GetExtent();

		std::array<VkClearValue, 2> clearColors;
		clearColors[0].color = { 0.1f, 0.1f, 0.1f, 1.f };
		clearColors[1].depthStencil = { 1.f, 0 };
		renderPassBegin.clearValueCount = 2;
		renderPassBegin.pClearValues = clearColors.data();

		vkCmdBeginRenderPass(static_cast<VkCommandBuffer>(s_pTempStorage->commandBuffer->GetCurrentCommandBuffer()), &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
		s_pTempStorage->mainPipeline->Bind(currentFrame);
	}

	void Renderer::End()
	{
		//Renderer3D::End();
	
		vkCmdEndRenderPass(static_cast<VkCommandBuffer>(s_pTempStorage->commandBuffer->GetCurrentCommandBuffer()));
		s_pTempStorage->commandBuffer->End();
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

	void Renderer::SetupBuffers()
	{
		s_pTempStorage->uniformBuffer.model = glm::scale(glm::mat4(1.f), { 0.01f, 0.01f, 0.01f }) * glm::rotate(glm::mat4(1.f), glm::radians(90.f), { 1.f, 0.f, 0.f });
		s_pTempStorage->uniformBuffer.view = glm::lookAt(glm::vec3{ 2.f, 2.f, 2.f }, glm::vec3{ 0.f, 0.f, 0.f }, glm::vec3{ 0.f, 0.f, 1.f });
		s_pTempStorage->uniformBuffer.projection = glm::perspective(glm::radians(45.f), 16.f / 9.f, 0.1f, 100.f);

		s_pTempStorage->uniformBufferSet = UniformBufferSet::Create(GetCapabilities().framesInFlight);
		s_pTempStorage->uniformBufferSet->Add(&s_pTempStorage->uniformBuffer, sizeof(TestUniformBuffer), 0, 0);
	}

	void* Renderer::GetDescriptorPool()
	{
		return s_descriptorPool;
	}

	void Renderer::Draw()
	{
		const uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		auto vulkanPipeline = std::dynamic_pointer_cast<VulkanRenderPipeline>(s_pTempStorage->mainPipeline);

		vulkanPipeline->SetTexture(s_pTempStorage->teddyTexture, 1, 0, currentFrame);
		vulkanPipeline->BindDescriptorSets(currentFrame);

		for (const auto subMesh : s_pTempStorage->teddy->GetSubMeshes())
		{
			subMesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(s_pTempStorage->commandBuffer);
			subMesh->GetVertexArray()->GetIndexBuffer()->Bind(s_pTempStorage->commandBuffer);

			vkCmdDrawIndexed(static_cast<VkCommandBuffer>(s_pTempStorage->commandBuffer->GetCurrentCommandBuffer()), subMesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
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
			s_pSceneData->ssaoNoise.push_back(noise);
		}
	}
}