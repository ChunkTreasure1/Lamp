#include "lppch.h"
#include "VulkanRenderer.h"

#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Shader/Shader.h"
#include "Lamp/Rendering/CommandBuffer.h"
#include "Lamp/Rendering/Textures/Texture2D.h"
#include "Lamp/Rendering/Renderer.h"

#include "Lamp/Mesh/Mesh.h"
#include "Lamp/AssetSystem/MeshImporter.h"
#include "Lamp/Rendering/Cameras/CameraBase.h"
#include "Lamp/Level/Level.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanRenderPipeline.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanTextureCube.h"
#include "Platform/Vulkan/VulkanUtility.h"
#include "Platform/Vulkan/VulkanRenderComputePipeline.h"
#include "Platform/Vulkan/VulkanShaderStorageBuffer.h"

#include "Lamp/World/Terrain.h"

#define ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR))))

#define PER_PASS_DESCRIPTOR_SET 0
#define PER_MESH_DESCRIPTOR_SET 1
#define MAX_DIRECTIONAL_LIGHT_SHADOWS 10

namespace Lamp
{
	VulkanRenderer::VulkanRenderer()
	{
		m_rendererStorage = CreateScope<VulkanRendererStorage>();

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
		poolInfo.poolSizeCount = (uint32_t)ARRAYSIZE(poolSizes);
		poolInfo.pPoolSizes = poolSizes;

		m_descriptorPools.resize(Renderer::GetCapabilities().framesInFlight);
		auto device = VulkanContext::GetCurrentDevice();

		for (uint32_t i = 0; i < m_descriptorPools.size(); ++i)
		{
			LP_VK_CHECK(vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &m_descriptorPools[i]));
		}
	}

	VulkanRenderer::~VulkanRenderer()
	{
		auto device = VulkanContext::GetCurrentDevice();
		vkDeviceWaitIdle(device->GetHandle());

		for (uint32_t i = 0; i < m_descriptorPools.size(); ++i)
		{
			vkDestroyDescriptorPool(device->GetHandle(), m_descriptorPools[i], nullptr);
		}

		m_descriptorPools.clear();
	}

	void VulkanRenderer::Initialize()
	{
		m_rendererStorage->swapchainCommandBuffer = CommandBuffer::Create(3, true);
		m_rendererStorage->renderCommandBuffer = CommandBuffer::Create(3, false);

		m_rendererStorage->quadMesh = SubMesh::CreateQuad();
	}

	void VulkanRenderer::Shutdown()
	{
	}

	void VulkanRenderer::Begin(const Ref<CameraBase> camera)
	{
		m_rendererStorage->camera = camera;

		auto swapchain = std::reinterpret_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());
		const uint32_t currentFrame = swapchain->GetCurrentFrame();

		//Reset descriptor pool
		vkResetDescriptorPool(VulkanContext::GetCurrentDevice()->GetHandle(), m_descriptorPools[currentFrame], 0);
	}

	void VulkanRenderer::End()
	{
		m_rendererStorage->camera = nullptr;
	}

	void VulkanRenderer::BeginPass(Ref<RenderPipeline> pipeline)
	{
		m_rendererStorage->currentRenderPipeline = pipeline;

		auto swapchain = std::reinterpret_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());
		auto framebuffer = std::reinterpret_pointer_cast<VulkanFramebuffer>(pipeline->GetSpecification().framebuffer);
		auto commandBuffer = pipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;
		const uint32_t currentFrame = swapchain->GetCurrentFrame();

		commandBuffer->Begin();

		VkRenderPassBeginInfo renderPassBegin{};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.renderPass = framebuffer->GetRenderPass();
		renderPassBegin.framebuffer = pipeline->GetSpecification().isSwapchain ? swapchain->GetFramebuffer(currentFrame) : framebuffer->GetFramebuffer();
		renderPassBegin.renderArea.offset = { 0, 0 };

		VkExtent2D extent;
		extent.width = framebuffer->GetSpecification().width;
		extent.height = framebuffer->GetSpecification().height;

		renderPassBegin.renderArea.extent = extent;

		renderPassBegin.clearValueCount = framebuffer->GetClearValues().size();
		renderPassBegin.pClearValues = framebuffer->GetClearValues().data();

		vkCmdBeginRenderPass(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

		UpdatePerPassDescriptors();
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(pipeline);
		vulkanPipeline->BindDescriptorSet(commandBuffer, m_rendererStorage->pipelineDescriptorSets[PER_PASS_DESCRIPTOR_SET], PER_PASS_DESCRIPTOR_SET);
	}

	void VulkanRenderer::EndPass()
	{
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		vkCmdEndRenderPass(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()));
		commandBuffer->End();

		m_rendererStorage->currentRenderPipeline = nullptr;
	}

	const GPUMemoryStatistics& VulkanRenderer::GetMemoryUsage() const
	{
		return VulkanAllocator::GetStatistics();
	}

	void VulkanRenderer::SubmitMesh(const glm::mat4& transform, const Ref<SubMesh> mesh, const Ref<Material> material, size_t id)
	{
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;
		auto vulkanMaterial = std::reinterpret_pointer_cast<VulkanMaterial>(material);

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		vulkanPipeline->Bind(commandBuffer);

		SetupDescriptorsForMaterialRendering(material);
		vulkanPipeline->BindDescriptorSets(commandBuffer, m_rendererStorage->currentMeshDescriptorSets);

		const auto& matData = material->GetMaterialData();

		MeshDataBuffer meshData;
		meshData.blendingUseBlending.x = matData.blendingMultiplier;
		meshData.blendingUseBlending.y = matData.useBlending;

		meshData.useAlbedo = matData.useAlbedo;
		meshData.useNormal = matData.useNormal;
		meshData.useMRO = matData.useMRO;

		meshData.mroColor = matData.mroColor;
		meshData.albedoColor = matData.albedoColor;
		meshData.normalColor = matData.normalColor;

		meshData.useSkybox = false;

		if (LevelManager::IsLevelLoaded() && LevelManager::GetActive()->HasSkybox())
		{
			meshData.useSkybox = true;
		}

		vulkanPipeline->SetPushConstantData(commandBuffer, 0, &transform);
		vulkanPipeline->SetPushConstantData(commandBuffer, 1, &meshData);

		mesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(commandBuffer);
		mesh->GetVertexArray()->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), mesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void VulkanRenderer::SubmitMesh(const Ref<SubMesh> mesh, const Ref<Material> material, const std::vector<VkDescriptorSet>& descriptorSets, void* pushConstant)
	{
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		vulkanPipeline->Bind(commandBuffer);
		vulkanPipeline->BindDescriptorSets(commandBuffer, descriptorSets);

		if (pushConstant)
		{
			vulkanPipeline->SetPushConstantData(commandBuffer, 0, pushConstant);
		}

		mesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(commandBuffer);
		mesh->GetVertexArray()->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), mesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void VulkanRenderer::SubmitQuad()
	{
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		vulkanPipeline->Bind(commandBuffer);
		SetupDescriptorsForQuadRendering();

		vulkanPipeline->BindDescriptorSets(commandBuffer, m_rendererStorage->pipelineDescriptorSets);

		m_rendererStorage->quadMesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(commandBuffer);
		m_rendererStorage->quadMesh->GetVertexArray()->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), m_rendererStorage->quadMesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void VulkanRenderer::DrawBuffer(RenderBuffer& buffer)
	{
		if (m_rendererStorage->currentRenderPipeline->GetSpecification().drawSkybox && LevelManager::GetActive()->HasSkybox())
		{
			auto pipeline = m_rendererStorage->currentRenderPipeline;
			m_rendererStorage->currentRenderPipeline = LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->GetPipeline();
			LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->Draw();

			m_rendererStorage->currentRenderPipeline = pipeline;
		}

		if (m_rendererStorage->currentRenderPipeline->GetSpecification().drawTerrain && LevelManager::GetActive()->HasTerrain())
		{
			auto pipeline = m_rendererStorage->currentRenderPipeline;

			m_rendererStorage->currentRenderPipeline = LevelManager::GetActive()->GetEnvironment().GetTerrain().terrain->GetPipeline();
			LevelManager::GetActive()->GetEnvironment().GetTerrain().terrain->Draw();

			m_rendererStorage->currentRenderPipeline = pipeline;
		}

		switch (m_rendererStorage->currentRenderPipeline->GetSpecification().drawType)
		{
			case DrawType::Buffer:
			{
				for (const auto& command : buffer.drawCalls)
				{
					SubmitMesh(command.transform, command.data, command.material, command.id);
				}

				break;
			}

			case DrawType::Terrain:
			{
				LevelManager::GetActive()->GetEnvironment().GetTerrain().terrain->Draw();

				break;
			}

			case DrawType::Quad:
			{
				SubmitQuad();

				break;
			}
		}
	}

	VkDescriptorSet VulkanRenderer::AllocateDescriptorSet(VkDescriptorSetAllocateInfo& allocInfo)
	{
		auto device = VulkanContext::GetCurrentDevice();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();
		
		allocInfo.descriptorPool = m_descriptorPools[currentFrame];

		VkDescriptorSet descriptorSet;
		LP_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, &descriptorSet));

		return descriptorSet;
	}

	//TODO: Find better way to save execute function
	std::pair<Ref<RenderComputePipeline>, std::function<void()>> VulkanRenderer::CreateLightCullingPipeline(Ref<UniformBuffer> cameraDataBuffer, Ref<UniformBuffer> lightCullingBuffer, Ref<ShaderStorageBufferSet> shaderStorageSet, Ref<Image2D> depthImage)
	{
		Ref<Shader> lightCullingShader = ShaderLibrary::GetShader("lightCulling");
		Ref<VulkanRenderComputePipeline> lightCullingPipeline = std::reinterpret_pointer_cast<VulkanRenderComputePipeline>(RenderComputePipeline::Create(lightCullingShader));

		Ref<VulkanShader> vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(lightCullingShader);

		auto func = [=]()
		{
			auto device = VulkanContext::GetCurrentDevice();
			uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

			std::array<VkWriteDescriptorSet, 5> writeDescriptors;
			auto descriptorSet = vulkanShader->CreateDescriptorSets();

			auto vulkanCameraDataBuffer = std::reinterpret_pointer_cast<VulkanUniformBuffer>(cameraDataBuffer);
			writeDescriptors[0] = *vulkanShader->GetDescriptorSet("CameraDataBuffer");
			writeDescriptors[0].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[0].pBufferInfo = &vulkanCameraDataBuffer->GetDescriptorInfo();

			auto vulkanLightCullingBuffer = std::reinterpret_pointer_cast<VulkanUniformBuffer>(lightCullingBuffer);
			writeDescriptors[1] = *vulkanShader->GetDescriptorSet("LightCullingBuffer");
			writeDescriptors[1].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[1].pBufferInfo = &vulkanLightCullingBuffer->GetDescriptorInfo();

			auto vulkanLightStorageBuffer = std::reinterpret_pointer_cast<VulkanShaderStorageBuffer>(shaderStorageSet->Get(12, 0, currentFrame));
			writeDescriptors[2] = *vulkanShader->GetDescriptorSet("LightBuffer");
			writeDescriptors[2].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[2].pBufferInfo = &vulkanLightStorageBuffer->GetDescriptorInfo();

			auto vulkanVisibleLightsBuffer = std::reinterpret_pointer_cast<VulkanShaderStorageBuffer>(shaderStorageSet->Get(13, 0, currentFrame));
			writeDescriptors[3] = *vulkanShader->GetDescriptorSet("VisibleLightsBuffer");
			writeDescriptors[3].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[3].pBufferInfo = &vulkanVisibleLightsBuffer->GetDescriptorInfo();

			auto vulkanDepthImage = std::reinterpret_pointer_cast<VulkanImage2D>(depthImage);
			writeDescriptors[4] = *vulkanShader->GetDescriptorSet("u_DepthMap");
			writeDescriptors[4].dstSet = descriptorSet.descriptorSets[0];
			writeDescriptors[4].pImageInfo = &vulkanDepthImage->GetDescriptorInfo();

			vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
			lightCullingPipeline->Execute(descriptorSet.descriptorSets.data(), (uint32_t)descriptorSet.descriptorSets.size(), Renderer::GetSceneData()->screenGroupX, Renderer::GetSceneData()->screenGroupY, 1);
		};

		return std::make_pair(lightCullingPipeline, func);
	}

	Ref<VulkanRenderer> VulkanRenderer::Create()
	{
		return CreateRef<VulkanRenderer>();
	}

	void VulkanRenderer::SetupDescriptorsForMaterialRendering(Ref<Material> material)
	{
		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_rendererStorage->currentRenderPipeline->GetSpecification().shader);
		auto vulkanMaterial = std::reinterpret_pointer_cast<VulkanMaterial>(material);
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);

		auto device = VulkanContext::GetCurrentDevice();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		m_rendererStorage->currentMeshDescriptorSets.clear();
		m_rendererStorage->currentMeshDescriptorSets.emplace_back(m_rendererStorage->pipelineDescriptorSets[PER_PASS_DESCRIPTOR_SET]);

		//Make sure that the shader has a descriptor set number 1
		if (vulkanShader->GetDescriptorSetLayoutCount() < PER_MESH_DESCRIPTOR_SET + 1)
		{
			return;
		}

		auto descriptorLayout = vulkanShader->GetDescriptorSetLayout(PER_MESH_DESCRIPTOR_SET);
		auto& shaderDescriptorSet = vulkanShader->GetDescriptorSets()[PER_MESH_DESCRIPTOR_SET];

		//Allocate descriptors
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPools[currentFrame];
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorLayout;

		auto& currentDescriptorSet = m_rendererStorage->currentMeshDescriptorSets.emplace_back();

		LP_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, &currentDescriptorSet));

		m_rendererStorage->pipelineDescriptorSets.emplace_back(currentDescriptorSet);

		//Update
		std::vector<VkWriteDescriptorSet> writeDescriptors;

		auto& textureSpecification = vulkanMaterial->GetTextureSpecification();
		for (const auto& spec : textureSpecification)
		{
			if (spec.set != 1) // TODO: should only material texture descriptors be in material?
			{
				continue;
			}

			Ref<VulkanTexture2D> vulkanTexture;

			if (spec.texture)
			{
				vulkanTexture = std::reinterpret_pointer_cast<VulkanTexture2D>(spec.texture);
			}
			else
			{
				vulkanTexture = std::reinterpret_pointer_cast<VulkanTexture2D>(Renderer::GetSceneData()->whiteTexture);
			}

			auto& imageSamplers = shaderDescriptorSet.imageSamplers;

			VulkanShader::ImageSampler* sampler = nullptr;

			auto imageNameIt = imageSamplers.find(spec.binding);
			if (imageNameIt != imageSamplers.end())
			{
				sampler = const_cast<VulkanShader::ImageSampler*>(&imageNameIt->second);
			}

			if (sampler)
			{
				auto descriptorWrite = shaderDescriptorSet.writeDescriptorSets.at(sampler->name);
				descriptorWrite.dstSet = currentDescriptorSet;
				descriptorWrite.pImageInfo = &vulkanTexture->GetDescriptorInfo();

				writeDescriptors.emplace_back(descriptorWrite);
			}
		}

		vkUpdateDescriptorSets(device->GetHandle(), static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
	}

	void VulkanRenderer::SetupDescriptorsForQuadRendering()
	{
		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_rendererStorage->currentRenderPipeline->GetSpecification().shader);
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);

		auto allDescriptorLayouts = vulkanShader->GetAllDescriptorSetLayouts();
		auto device = VulkanContext::GetCurrentDevice();

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		/////Allocate//////
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPools[currentFrame];
		allocInfo.descriptorSetCount = static_cast<uint32_t>(allDescriptorLayouts.size());
		allocInfo.pSetLayouts = allDescriptorLayouts.data();

		auto& currentDescriptorSet = m_rendererStorage->pipelineDescriptorSets;
		currentDescriptorSet.resize(allDescriptorLayouts.size());

		LP_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, currentDescriptorSet.data()));

		/////Update/////
		auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();

		if (!shaderDescriptorSets.empty())
		{
			//Uniform buffers
			for (uint32_t set = 0; set < shaderDescriptorSets.size(); set++)
			{
				auto& uniformBuffers = shaderDescriptorSets[set].uniformBuffers;

				for (const auto& uniformBuffer : uniformBuffers)
				{
					auto writeDescriptor = shaderDescriptorSets[set].writeDescriptorSets.at(uniformBuffer.second->name);
					writeDescriptor.dstSet = currentDescriptorSet[set];

					auto vulkanUniformBuffer = std::reinterpret_pointer_cast<VulkanUniformBuffer>(vulkanPipeline->GetSpecification().uniformBufferSets->Get(uniformBuffer.second->bindPoint, set, currentFrame));
					writeDescriptor.pBufferInfo = &vulkanUniformBuffer->GetDescriptorInfo();

					vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
				}
			}

			//Framebuffer textures
			auto& framebufferInputs = m_rendererStorage->currentRenderPipeline->GetSpecification().framebufferInputs;
			for (const auto& framebufferInput : framebufferInputs)
			{
				if (framebufferInput.attachment)
				{
					auto vulkanImage = std::reinterpret_pointer_cast<VulkanImage2D>(framebufferInput.attachment);
					auto& imageSamplers = shaderDescriptorSets[framebufferInput.set].imageSamplers;

					auto imageSampler = imageSamplers.find(framebufferInput.binding);
					if (imageSampler != imageSamplers.end())
					{
						auto descriptorWrite = shaderDescriptorSets[framebufferInput.set].writeDescriptorSets.at(imageSampler->second.name);
						descriptorWrite.dstSet = currentDescriptorSet[framebufferInput.set];
						descriptorWrite.pImageInfo = &vulkanImage->GetDescriptorInfo();

						auto device = VulkanContext::GetCurrentDevice();
						vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
					}
				}
				else
				{ 
					LP_CORE_ERROR("VulkanRenderer: No attachment bound to binding {0} in pipeline {1}!", framebufferInput.binding, "");
				}
			}
 		}
	}

	void VulkanRenderer::UpdatePerPassDescriptors()		
	{
		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_rendererStorage->currentRenderPipeline->GetSpecification().shader);
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);

		auto device = VulkanContext::GetCurrentDevice();
		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		LP_ASSERT(vulkanShader->GetDescriptorSetLayoutCount() > 0, "Descriptor set layouts must be greater than zero!");
		LP_ASSERT(!vulkanShader->GetDescriptorSets().empty(), "Descriptor set count must be greater than zero!");

		auto descriptorLayout = vulkanShader->GetDescriptorSetLayout(PER_PASS_DESCRIPTOR_SET);
		auto& shaderDescriptorSet = vulkanShader->GetDescriptorSets()[PER_PASS_DESCRIPTOR_SET];

		//Allocate descriptors
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorLayout;

		m_rendererStorage->pipelineDescriptorSets.clear();
		auto& currentDescriptorSet = m_rendererStorage->pipelineDescriptorSets.emplace_back();
		currentDescriptorSet = AllocateDescriptorSet(allocInfo);

		//Update descriptors
		std::vector<VkWriteDescriptorSet> writeDescriptors;

		auto& uniformBuffers = shaderDescriptorSet.uniformBuffers;

		//Uniform buffers
		for (const auto& uniformBuffer : uniformBuffers)
		{
			auto writeDescriptor = shaderDescriptorSet.writeDescriptorSets.at(uniformBuffer.second->name);
			writeDescriptor.dstSet = currentDescriptorSet;

			auto vulkanUniformBuffer = std::reinterpret_pointer_cast<VulkanUniformBuffer>(vulkanPipeline->GetSpecification().uniformBufferSets->Get(uniformBuffer.second->bindPoint, 0, currentFrame));
			writeDescriptor.pBufferInfo = &vulkanUniformBuffer->GetDescriptorInfo();

			writeDescriptors.emplace_back(writeDescriptor);
		}

		auto& shaderBuffers = shaderDescriptorSet.storageBuffers;
		//Shader buffers
		for (const auto& shaderBuffer : shaderBuffers)
		{
			auto writeDescriptor = shaderDescriptorSet.writeDescriptorSets.at(shaderBuffer.second->name);
			writeDescriptor.dstSet = currentDescriptorSet;

			auto vulkanShaderBuffer = std::reinterpret_pointer_cast<VulkanShaderStorageBuffer>(vulkanPipeline->GetSpecification().shaderStorageBufferSets->Get(shaderBuffer.second->bindPoint, 0, currentFrame));
			writeDescriptor.pBufferInfo = &vulkanShaderBuffer->GetDescriptorInfo();

			writeDescriptors.emplace_back(writeDescriptor);
		}

		std::vector<VkDescriptorImageInfo> descriptorInfos;
		descriptorInfos.reserve(10);

		//Directional lights
		if (LevelManager::GetActive())
		{
			auto it = shaderDescriptorSet.writeDescriptorSets.find("u_DirShadowMaps");
			if (it != shaderDescriptorSet.writeDescriptorSets.end())
			{
				auto writeDescriptor = it->second;

				writeDescriptor.dstSet = currentDescriptorSet;

				uint32_t i = 0;
				for (auto light : LevelManager::GetActive()->GetEnvironment().GetDirectionalLights())
				{
					if (!light->castShadows)
					{
						continue;
					}

					if (i >= MAX_DIRECTIONAL_LIGHT_SHADOWS)
					{
						break;
					}

					descriptorInfos.emplace_back(std::reinterpret_pointer_cast<VulkanImage2D>(light->shadowBuffer->GetDepthAttachment())->GetDescriptorInfo());

					i++;
				}

				writeDescriptor.descriptorCount = descriptorInfos.size();
				writeDescriptor.pImageInfo = descriptorInfos.data();

				if (writeDescriptor.descriptorCount > 0)
				{
					writeDescriptors.emplace_back(writeDescriptor);
				}
			}
		}

		//Framebuffer textures
		auto& framebufferInputs = m_rendererStorage->currentRenderPipeline->GetSpecification().framebufferInputs;
		for (const auto& framebufferInput : framebufferInputs)
		{
			Ref<VulkanImage2D> vulkanImage;

			if (framebufferInput.attachment)
			{
				vulkanImage = std::reinterpret_pointer_cast<VulkanImage2D>(framebufferInput.attachment);
			}
			else
			{
				vulkanImage = std::reinterpret_pointer_cast<VulkanTexture2D>(Renderer::GetSceneData()->whiteTexture)->GetImage();
			}

			auto& imageSamplers = shaderDescriptorSet.imageSamplers;

			auto imageSampler = imageSamplers.find(framebufferInput.binding);
			if (imageSampler != imageSamplers.end())
			{
				auto descriptorWrite = shaderDescriptorSet.writeDescriptorSets.at(imageSampler->second.name);
				descriptorWrite.dstSet = currentDescriptorSet;
				descriptorWrite.pImageInfo = &vulkanImage->GetDescriptorInfo();

				writeDescriptors.emplace_back(descriptorWrite);
			}
		}

		if (LevelManager::GetActive())
		{
			auto irradiance = shaderDescriptorSet.writeDescriptorSets.find("u_IrradianceMap");
			if (irradiance != shaderDescriptorSet.writeDescriptorSets.end())
			{
				auto writeDescriptor = irradiance->second;
				writeDescriptor.dstSet = currentDescriptorSet;
				writeDescriptor.descriptorCount = 1;

				if (LevelManager::GetActive()->HasSkybox())
				{
					writeDescriptor.pImageInfo = &std::reinterpret_pointer_cast<VulkanTextureCube>(LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->GetIrradiance())->GetDescriptorInfo();
				}
				else
				{
					writeDescriptor.pImageInfo = &std::reinterpret_pointer_cast<VulkanTextureCube>(Renderer::GetSceneData()->blackCubeTexture)->GetDescriptorInfo();
				}

				writeDescriptors.emplace_back(writeDescriptor);
			}

			auto prefilter = shaderDescriptorSet.writeDescriptorSets.find("u_PrefilterMap");
			if (prefilter != shaderDescriptorSet.writeDescriptorSets.end())
			{
				auto writeDescriptor = prefilter->second;
				writeDescriptor.dstSet = currentDescriptorSet;
				writeDescriptor.descriptorCount = 1;

				if (LevelManager::GetActive()->HasSkybox())
				{
					writeDescriptor.pImageInfo = &std::reinterpret_pointer_cast<VulkanTextureCube>(LevelManager::GetActive()->GetEnvironment().GetSkybox().skybox->GetFilteredEnvironment())->GetDescriptorInfo();
				}
				else
				{
					writeDescriptor.pImageInfo = &std::reinterpret_pointer_cast<VulkanTextureCube>(Renderer::GetSceneData()->blackCubeTexture)->GetDescriptorInfo();
				}

				writeDescriptors.emplace_back(writeDescriptor);
			}
		}

		vkUpdateDescriptorSets(device->GetHandle(), (uint32_t)writeDescriptors.size(), writeDescriptors.data(), 0, nullptr);
	}
}