#include "lppch.h"
#include "VulkanRenderPipeline.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanTexture2D.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanUtility.h"

#include "Lamp/Core/Application.h"
#include "Lamp/Rendering/CommandBuffer.h"

namespace Lamp
{
	namespace Utils
	{
		static VkFormat FormatFromShaderIputType(ElementType type)
		{
			switch (type)
			{
				case ElementType::Bool: return VK_FORMAT_R8_UINT;
				case ElementType::Int: return VK_FORMAT_R8_SINT;
				case ElementType::Float: return VK_FORMAT_R32_SFLOAT;
				case ElementType::Float2: return VK_FORMAT_R32G32_SFLOAT;
				case ElementType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
				case ElementType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
				case ElementType::Mat3: return VK_FORMAT_R8_UNORM;
				case ElementType::Mat4: return VK_FORMAT_R8_UNORM;
			}

			return VK_FORMAT_R8_UNORM;
		}

		static VkPrimitiveTopology VulkanTopologyFromTopology(Topology topology)
		{
			switch (topology)
			{
				case Topology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				case Topology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
				case Topology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			}

			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		}
	}

	VulkanRenderPipeline::VulkanRenderPipeline(const RenderPipelineSpecification& specification)
		: m_specification(specification)
	{
		SetLayout(specification.vertexLayout);
	}

	VulkanRenderPipeline::~VulkanRenderPipeline()
	{
		auto device = VulkanContext::GetCurrentDevice();

		vkDeviceWaitIdle(device->GetHandle());

		vkDestroyPipelineLayout(device->GetHandle(), m_layout, nullptr);
		vkDestroyPipeline(device->GetHandle(), m_pipeline, nullptr);
	}

	void VulkanRenderPipeline::Bind(Ref<CommandBuffer> commandBuffer) const
	{
		auto vulkanCommandBuffer = static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer());

		VkExtent2D extent{};
		extent.width = m_specification.framebuffer->GetSpecification().width;
		extent.height = m_specification.framebuffer->GetSpecification().height;

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = 0.f;
		viewport.width = extent.width;
		viewport.height = extent.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor = { { 0, 0 }, extent };
		vkCmdSetViewport(vulkanCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(vulkanCommandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(vulkanCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
	}

	void VulkanRenderPipeline::SetLayout(BufferLayout layout)
	{
		auto device = VulkanContext::GetCurrentDevice();

		if (m_pipeline != VK_NULL_HANDLE && m_layout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(device->GetHandle(), m_layout, nullptr);
			vkDestroyPipeline(device->GetHandle(), m_pipeline, nullptr);
		}

		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(Vertex);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		m_bindingDescription = bindingDesc;

		m_numAttributes = 0;

		for (const auto& attr : layout.GetElements())
		{
			VkVertexInputAttributeDescription desc{};
			desc.binding = 0;
			desc.location = m_numAttributes;
			desc.format = Utils::FormatFromShaderIputType(attr.ElementType);
			desc.offset = attr.Offset;

			m_attributeDescriptions.push_back(desc);

			m_numAttributes++;
		}

		Invalidate();
	}
	   
	void VulkanRenderPipeline::SetTexture(Ref<Texture2D> texture, uint32_t binding, uint32_t set, uint32_t index)
	{
		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_specification.shader);
		auto vulkanTexture = std::reinterpret_pointer_cast<VulkanTexture2D>(texture);

		LP_CORE_ASSERT(index < m_descriptorSets.size(), "Index must be less than the descriptor set map size!");

		auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
		auto& imageSamplers = shaderDescriptorSets[set].imageSamplers;
		auto descriptorWrite = shaderDescriptorSets[set].writeDescriptorSets.at(imageSamplers.at(binding).name);
		descriptorWrite.dstSet = m_descriptorSets[index][set];
		descriptorWrite.pImageInfo = &vulkanTexture->GetDescriptorInfo();

		auto device = VulkanContext::GetCurrentDevice();
		vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
	}

	void VulkanRenderPipeline::SetTexture(Ref<Image2D> image, uint32_t set, uint32_t binding, uint32_t index)
	{
		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_specification.shader);
		auto vulkanImage = std::reinterpret_pointer_cast<VulkanImage2D>(image);
		
		//TODO: transition to right layout
		Utility::TransitionImageLayout(vulkanImage->GetHandle(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();
		auto& imageSamplers = shaderDescriptorSets[set].imageSamplers;
		auto descriptorWrite = shaderDescriptorSets[set].writeDescriptorSets.at(imageSamplers.at(binding).name);
		descriptorWrite.dstSet = m_descriptorSets[index][set];
		descriptorWrite.pImageInfo = &vulkanImage->GetDescriptorInfo();

		auto device = VulkanContext::GetCurrentDevice();
		vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
	}

	void VulkanRenderPipeline::SetPushConstantData(Ref<CommandBuffer> commandBuffer, uint32_t index, const void* data)
	{
		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_specification.shader);
		auto vulkanCommandBuffer = reinterpret_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer());

		const auto& pushConstants = vulkanShader->GetPushConstantRanges();

		vkCmdPushConstants(vulkanCommandBuffer, m_layout, pushConstants[index].shaderStage, pushConstants[index].offset, pushConstants[index].size, data);
	}

	void VulkanRenderPipeline::CreateDescriptorSets()
	{
		auto swapchain = std::reinterpret_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());
		auto device = VulkanContext::GetCurrentDevice();

		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_specification.shader);

		auto allDescriptorLayouts = vulkanShader->GetAllDescriptorSetLayouts();

		const uint32_t framesInFlight = m_specification.isSwapchain ? swapchain->GetImageCount() : 1;

		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = static_cast<VkDescriptorPool>(std::reinterpret_pointer_cast<VulkanRenderer>(Renderer::GetRenderer())->GetDescriptorPool());
			allocInfo.descriptorSetCount = static_cast<uint32_t>(allDescriptorLayouts.size());
			allocInfo.pSetLayouts = allDescriptorLayouts.data();

			m_descriptorSets[i].resize(allDescriptorLayouts.size());

			VkResult result = vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, m_descriptorSets[i].data());
			LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create descriptor sets!");
		}
	}

	void VulkanRenderPipeline::SetupUniformBuffers()
	{
		auto device = VulkanContext::GetCurrentDevice();
		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_specification.shader);

		const uint32_t count = m_specification.isSwapchain ? Renderer::GetCapabilities().framesInFlight : 1;
		auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();

		for (uint32_t frame = 0; frame < count; frame++)
		{
			for (uint32_t set = 0; set < shaderDescriptorSets.size(); set++)
			{
				auto& uniformBuffers = shaderDescriptorSets[set].uniformBuffers;

				for (uint32_t binding = 0; binding < uniformBuffers.size(); binding++)
				{
					auto writeDescriptor = shaderDescriptorSets[set].writeDescriptorSets.at(uniformBuffers.at(binding)->name);
					writeDescriptor.dstSet = m_descriptorSets[frame][set];

					auto vulkanUniformBuffer = std::reinterpret_pointer_cast<VulkanUniformBuffer>(m_specification.uniformBufferSets->Get(binding, set, frame));
					writeDescriptor.pBufferInfo = &vulkanUniformBuffer->GetDescriptorInfo();

					vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
				}
			}
		}
	}

	void VulkanRenderPipeline::Invalidate()
	{
		auto device = VulkanContext::GetCurrentDevice();
		auto swapchain = std::reinterpret_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());
		auto shader = std::reinterpret_pointer_cast<VulkanShader>(m_specification.shader);
		Ref<VulkanFramebuffer> framebuffer = nullptr;
		
		if (m_specification.framebuffer)
		{
			framebuffer = std::reinterpret_pointer_cast<VulkanFramebuffer>(m_specification.framebuffer);
		}

		CreateDescriptorSets();
		SetupUniformBuffers();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = m_attributeDescriptions.empty() ? 0 : 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &m_bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = m_attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = Utils::VulkanTopologyFromTopology(m_specification.topology);
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = 2;

		VkDynamicState states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		dynamicStateInfo.pDynamicStates = states;

		const auto& descriptorLayouts = shader->GetAllDescriptorSetLayouts();
		const auto& pushConstantRanges = shader->GetAllPushConstantRanges();

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorLayouts.size();
		pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.size();
		pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

		VkResult result = vkCreatePipelineLayout(device->GetHandle(), &pipelineLayoutInfo, nullptr, &m_layout);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create pipeline layout!");

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shader->GetShaderStageInfos().size());
		pipelineInfo.pStages = shader->GetShaderStageInfos().data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = m_layout;
		pipelineInfo.renderPass = m_specification.isSwapchain ? swapchain->GetRenderPass() : framebuffer->GetRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pDynamicState = &dynamicStateInfo;

		result = vkCreateGraphicsPipelines(device->GetHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create pipeline!");
	}

	void VulkanRenderPipeline::BindDescriptorSets(Ref<CommandBuffer> commandBuffer, const std::vector<VkDescriptorSet>& descriptorSets) const
	{
		auto vulkanCommanBuffer = reinterpret_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer());
		vkCmdBindDescriptorSets(vulkanCommanBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
	}
}