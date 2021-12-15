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
#include "Platform/Vulkan/VulkanTextureHDR.h"
#include "Platform/Vulkan/VulkanUtility.h"

#define ARRAYSIZE(_ARR) ((int)(sizeof(_ARR) / sizeof(*(_ARR))))

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

		auto device = VulkanContext::GetCurrentDevice();
		VkResult result = vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &m_descriptorPool);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create descriptor pool!");
	}

	VulkanRenderer::~VulkanRenderer()
	{
		auto device = VulkanContext::GetCurrentDevice();
		vkDeviceWaitIdle(device->GetHandle());
		vkDestroyDescriptorPool(device->GetHandle(), m_descriptorPool, nullptr);
	}

	void VulkanRenderer::Initialize()
	{
		m_rendererStorage->swapchainCommandBuffer = CommandBuffer::Create(3, true);
		m_rendererStorage->renderCommandBuffer = CommandBuffer::Create(3, false);

		CreateBaseGeometry();
	}

	void VulkanRenderer::Shutdown()
	{
	}

	void VulkanRenderer::Begin(const Ref<CameraBase> camera)
	{
		m_rendererStorage->camera = camera;
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
		commandBuffer->Begin();

		const uint32_t currentFrame = swapchain->GetCurrentFrame();

		VkRenderPassBeginInfo renderPassBegin{};
		renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBegin.renderPass = framebuffer->GetRenderPass();
		renderPassBegin.framebuffer = pipeline->GetSpecification().isSwapchain ? swapchain->GetFramebuffer(currentFrame) : framebuffer->GetFramebuffer();
		renderPassBegin.renderArea.offset = { 0, 0 };

		VkExtent2D extent;
		extent.width = framebuffer->GetSpecification().width;
		extent.height = framebuffer->GetSpecification().height;

		renderPassBegin.renderArea.extent = extent;

		std::array<VkClearValue, 2> clearColors;
		clearColors[0].color = { 0.1f, 0.1f, 0.1f, 1.f };
		clearColors[1].color = { 1.f, 0 };
		renderPassBegin.clearValueCount = 2;
		renderPassBegin.pClearValues = clearColors.data();

		vkCmdBeginRenderPass(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
		pipeline->Bind(commandBuffer);
	}

	void VulkanRenderer::EndPass()
	{
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		vkCmdEndRenderPass(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()));
		commandBuffer->End();

		if (!m_rendererStorage->pipelineDescriptorSets.empty())
		{
			auto device = VulkanContext::GetCurrentDevice();
			vkFreeDescriptorSets(device->GetHandle(), m_descriptorPool, m_rendererStorage->pipelineDescriptorSets.size(), m_rendererStorage->pipelineDescriptorSets.data());
			m_rendererStorage->pipelineDescriptorSets.clear();
		}

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

		SetupDescriptorsForMaterialRendering(material);

		m_rendererStorage->meshBuffer.model = transform;

		vulkanPipeline->SetPushConstantData(commandBuffer, 0, &m_rendererStorage->meshBuffer);
		vulkanPipeline->BindDescriptorSets(commandBuffer, vulkanMaterial->GetDescriptorSets()[currentFrame]);

		mesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(commandBuffer);
		mesh->GetVertexArray()->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), mesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void VulkanRenderer::SubmitQuad()
	{
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		SetupDescriptorsForQuadRendering();

		vulkanPipeline->BindDescriptorSets(commandBuffer, m_rendererStorage->pipelineDescriptorSets);

		m_rendererStorage->quadMesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(commandBuffer);
		m_rendererStorage->quadMesh->GetVertexArray()->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), m_rendererStorage->quadMesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void VulkanRenderer::SubmitCube(void* pushConstant)
	{
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);
		auto commandBuffer = m_rendererStorage->currentRenderPipeline->GetSpecification().isSwapchain ? m_rendererStorage->swapchainCommandBuffer : m_rendererStorage->renderCommandBuffer;

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		SetupDescriptorsForCubeRendering();

		if (pushConstant)
		{
			vulkanPipeline->SetPushConstantData(commandBuffer, 0, pushConstant);
		}

		vulkanPipeline->BindDescriptorSets(commandBuffer, m_rendererStorage->pipelineDescriptorSets);

		m_rendererStorage->cubeMesh->GetVertexArray()->GetVertexBuffers()[0]->Bind(commandBuffer);
		m_rendererStorage->cubeMesh->GetVertexArray()->GetIndexBuffer()->Bind(commandBuffer);

		vkCmdDrawIndexed(static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer()), m_rendererStorage->cubeMesh->GetVertexArray()->GetIndexBuffer()->GetCount(), 1, 0, 0, 0);
	}

	void VulkanRenderer::DrawBuffer(RenderBuffer& buffer)
	{
		switch (m_rendererStorage->currentRenderPipeline->GetSpecification().drawType)
		{
			case DrawType::Buffer:
			{
				if (m_rendererStorage->camera)
				{
					SortRenderBuffer(m_rendererStorage->camera->GetPosition(), buffer);
				}

				for (const auto& command : buffer.drawCalls)
				{
					SubmitMesh(command.transform, command.data, command.material, command.id);
				}

				break;
			}
		}
	}

	void VulkanRenderer::SetupDescriptorsForMaterialRendering(Ref<Material> material)
	{
		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(material->GetShader());
		auto vulkanMaterial = std::reinterpret_pointer_cast<VulkanMaterial>(material);
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);

		bool overrideShader = vulkanShader != m_rendererStorage->currentRenderPipeline->GetSpecification().shader;

		if (overrideShader)
		{
			vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_rendererStorage->currentRenderPipeline->GetSpecification().shader);
		}

		auto allDescriptorLayouts = vulkanShader->GetAllDescriptorSetLayouts();
		auto device = VulkanContext::GetCurrentDevice();

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		/////Allocate//////
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = static_cast<VkDescriptorPool>(std::reinterpret_pointer_cast<VulkanRenderer>(Renderer::GetRenderer())->GetDescriptorPool());
		allocInfo.descriptorSetCount = static_cast<uint32_t>(allDescriptorLayouts.size());
		allocInfo.pSetLayouts = allDescriptorLayouts.data();

		auto& currentDescriptorSet = vulkanMaterial->GetDescriptorSets()[currentFrame];

		if (!currentDescriptorSet.empty())
		{
			vkFreeDescriptorSets(device->GetHandle(), m_descriptorPool, currentDescriptorSet.size(), currentDescriptorSet.data());
		}

		currentDescriptorSet.resize(allDescriptorLayouts.size());

		VkResult result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, currentDescriptorSet.data());
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create descriptor sets!");

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

			//Material textures
			auto& textureSpecification = vulkanMaterial->GetTextureSpecification();
			for (const auto& spec : textureSpecification)
			{
				if (spec.texture)
				{
					auto vulkanTexture = std::reinterpret_pointer_cast<VulkanTexture2D>(spec.texture);
					auto& imageSamplers = shaderDescriptorSets[spec.set].imageSamplers;

					VulkanShader::ImageSampler* sampler = nullptr;

					if (overrideShader)
					{
						for (const auto& imageSampler : imageSamplers)
						{
							if (imageSampler.second.name == spec.name)
							{
								sampler = const_cast<VulkanShader::ImageSampler*>(&imageSampler.second);
							}
						}
					}
					else
					{
						auto imageNameIt = imageSamplers.find(spec.binding);
						if (imageNameIt != imageSamplers.end())
						{
							sampler = const_cast<VulkanShader::ImageSampler*>(&imageNameIt->second);
						}
					}

					if (sampler)
					{
						auto descriptorWrite = shaderDescriptorSets[spec.set].writeDescriptorSets.at(sampler->name);
						descriptorWrite.dstSet = currentDescriptorSet[spec.set];
						descriptorWrite.pImageInfo = &vulkanTexture->GetDescriptorInfo();

						auto device = VulkanContext::GetCurrentDevice();
						vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
					}
				}
				else
				{
					LP_CORE_ERROR("VulkanRenderer: No texture bound to {0} in material {1}!", spec.name, material->GetName());
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

			//Other textures
			auto& textureInputs = m_rendererStorage->currentRenderPipeline->GetSpecification().textureInputs;
			for (const auto& textureInput : textureInputs)
			{
				if (textureInput.texture)
				{
					auto vulkanImage = std::reinterpret_pointer_cast<VulkanTexture2D>(textureInput.texture)->GetImage();
					auto& imageSamplers = shaderDescriptorSets[textureInput.set].imageSamplers;

					auto imageSampler = imageSamplers.find(textureInput.binding);
					if (imageSampler != imageSamplers.end())
					{
						auto descriptorWrite = shaderDescriptorSets[textureInput.set].writeDescriptorSets.at(imageSampler->second.name);
						descriptorWrite.dstSet = currentDescriptorSet[textureInput.set];
						descriptorWrite.pImageInfo = &vulkanImage->GetDescriptorInfo();

						auto device = VulkanContext::GetCurrentDevice();
						vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
					}
				}
				else
				{
					LP_CORE_ERROR("VulkanRenderer: No texture bound to binding {0} in pipeline {1}!", textureInput.binding, "");
				}
			}

			auto& textureHDRInputs = m_rendererStorage->currentRenderPipeline->GetSpecification().textureHDRInputs;
			for (const auto& textureInput : textureHDRInputs)
			{
				if (textureInput.texture)
				{
					auto vulkanImage = std::reinterpret_pointer_cast<VulkanTextureHDR>(textureInput.texture)->GetImage();
					auto& imageSamplers = shaderDescriptorSets[textureInput.set].imageSamplers;

					auto imageSampler = imageSamplers.find(textureInput.binding);
					if (imageSampler != imageSamplers.end())
					{
						auto descriptorWrite = shaderDescriptorSets[textureInput.set].writeDescriptorSets.at(imageSampler->second.name);
						descriptorWrite.dstSet = currentDescriptorSet[textureInput.set];
						descriptorWrite.pImageInfo = &vulkanImage->GetDescriptorInfo();

						auto device = VulkanContext::GetCurrentDevice();
						vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
					}
				}
				else
				{
					LP_CORE_ERROR("VulkanRenderer: No texture bound to binding {0} in pipeline {1}!", textureInput.binding, "");
				}
			}
		}
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
		allocInfo.descriptorPool = static_cast<VkDescriptorPool>(std::reinterpret_pointer_cast<VulkanRenderer>(Renderer::GetRenderer())->GetDescriptorPool());
		allocInfo.descriptorSetCount = static_cast<uint32_t>(allDescriptorLayouts.size());
		allocInfo.pSetLayouts = allDescriptorLayouts.data();

		auto& currentDescriptorSet = m_rendererStorage->pipelineDescriptorSets;
		currentDescriptorSet.resize(allDescriptorLayouts.size());

		VkResult result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, currentDescriptorSet.data());
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create descriptor sets!");

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

			//Other textures
			auto& textureInputs = m_rendererStorage->currentRenderPipeline->GetSpecification().textureInputs;
			for (const auto& textureInput : textureInputs)
			{
				if (textureInput.texture)
				{
					auto vulkanImage = std::reinterpret_pointer_cast<VulkanTexture2D>(textureInput.texture)->GetImage();
					auto& imageSamplers = shaderDescriptorSets[textureInput.set].imageSamplers;

					auto imageSampler = imageSamplers.find(textureInput.binding);
					if (imageSampler != imageSamplers.end())
					{
						auto descriptorWrite = shaderDescriptorSets[textureInput.set].writeDescriptorSets.at(imageSampler->second.name);
						descriptorWrite.dstSet = currentDescriptorSet[textureInput.set];
						descriptorWrite.pImageInfo = &vulkanImage->GetDescriptorInfo();

						auto device = VulkanContext::GetCurrentDevice();
						vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
					}
				}
				else
				{
					LP_CORE_ERROR("VulkanRenderer: No texture bound to binding {0} in pipeline {1}!", textureInput.binding, "");
				}
			}
		}
	}

	void VulkanRenderer::SetupDescriptorsForCubeRendering()
	{
		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_rendererStorage->currentRenderPipeline->GetSpecification().shader);
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(m_rendererStorage->currentRenderPipeline);

		auto allDescriptorLayouts = vulkanShader->GetAllDescriptorSetLayouts();
		auto device = VulkanContext::GetCurrentDevice();

		uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();

		/////Allocate//////
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = static_cast<VkDescriptorPool>(std::reinterpret_pointer_cast<VulkanRenderer>(Renderer::GetRenderer())->GetDescriptorPool());
		allocInfo.descriptorSetCount = static_cast<uint32_t>(allDescriptorLayouts.size());
		allocInfo.pSetLayouts = allDescriptorLayouts.data();

		auto& currentDescriptorSet = m_rendererStorage->pipelineDescriptorSets;
		currentDescriptorSet.resize(allDescriptorLayouts.size());

		VkResult result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, currentDescriptorSet.data());
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create descriptor sets!");

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

			//Other textures
			auto& textureInputs = m_rendererStorage->currentRenderPipeline->GetSpecification().textureInputs;
			for (const auto& textureInput : textureInputs)
			{
				if (textureInput.texture)
				{
					auto vulkanImage = std::reinterpret_pointer_cast<VulkanTexture2D>(textureInput.texture)->GetImage();
					auto& imageSamplers = shaderDescriptorSets[textureInput.set].imageSamplers;

					auto imageSampler = imageSamplers.find(textureInput.binding);
					if (imageSampler != imageSamplers.end())
					{
						auto descriptorWrite = shaderDescriptorSets[textureInput.set].writeDescriptorSets.at(imageSampler->second.name);
						descriptorWrite.dstSet = currentDescriptorSet[textureInput.set];
						descriptorWrite.pImageInfo = &vulkanImage->GetDescriptorInfo();

						auto device = VulkanContext::GetCurrentDevice();
						vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
					}
				}
				else
				{
					LP_CORE_ERROR("VulkanRenderer: No texture bound to binding {0} in pipeline {1}!", textureInput.binding, "");
				}
			}
		}
	}

	void VulkanRenderer::SortRenderBuffer(const glm::vec3& sortPoint, RenderBuffer& buffer)
	{
		std::sort(buffer.drawCalls.begin(), buffer.drawCalls.end(), [&sortPoint](const RenderCommandData& dataOne, const RenderCommandData& dataTwo)
			{
				const glm::vec3& dPosOne = dataOne.transform[3];
				const glm::vec3& dPosTwo = dataTwo.transform[3];

				const float distOne = glm::exp2(sortPoint.x - dPosOne.x) + glm::exp2(sortPoint.y - dPosOne.y) + glm::exp2(sortPoint.z - dPosOne.z);
				const float distTwo = glm::exp2(sortPoint.x - dPosTwo.x) + glm::exp2(sortPoint.y - dPosTwo.y) + glm::exp2(sortPoint.z - dPosTwo.z);

				return distOne < distTwo;
			});
	}

	void VulkanRenderer::CreateBaseGeometry()
	{
		/////Quad/////
		{
			std::vector<Vertex> quadVertices =
			{
				Vertex({ -1.f, -1.f, 0.f }, { 0.f, 1.f }),
				Vertex({  1.f, -1.f, 0.f }, { 1.f, 1.f }),
				Vertex({  1.f,  1.f, 0.f }, { 1.f, 0.f }),
				Vertex({ -1.f,  1.f, 0.f }, { 0.f, 0.f }),
			};

			std::vector<uint32_t> quadIndices =
			{
				0, 1, 2,
				2, 3, 0
			};

			m_rendererStorage->quadMesh = CreateRef<SubMesh>(quadVertices, quadIndices, 0);
		}
		//////////////

		/////Cube/////
		{
			std::vector<Vertex> positions =
			{
				Vertex({-1, -1, -1}),
				Vertex({ 1, -1, -1}),
				Vertex({ 1,  1, -1}),
				Vertex({-1,  1, -1}),
				Vertex({-1, -1,  1}),
				Vertex({ 1, -1,  1}),
				Vertex({ 1,  1,  1}),
				Vertex({-1,  1,  1}),
			};

			std::vector<uint32_t> indices =
			{
				0, 1, 3,
				3, 1, 2,
				1, 5, 2,
				2, 5, 6,
				5, 4, 6,
				6, 4, 7,
				4, 0, 7,
				7, 0, 3,
				3, 2, 7,
				7, 2, 5,
				4, 5, 0,
				0, 5, 1
			};

			m_rendererStorage->cubeMesh = CreateRef<SubMesh>(positions, indices, 0);
		}
		//////////////
	}
}