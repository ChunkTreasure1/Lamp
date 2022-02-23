#include "lppch.h"
#include "RenderPipeline.h"

#include "Lamp/Core/Application.h"

#include "Lamp/Rendering/Textures/Image2D.h"
#include "Lamp/Rendering/Buffers/Framebuffer.h"
#include "Lamp/Rendering/Shader/Shader.h"
#include "Lamp/Rendering/Swapchain.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Rendering/Textures/Texture2D.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Lamp
{
	Ref<RenderPipeline> RenderPipeline::Create(const RenderPipelineSpecification& specification)
	{
		return CreateRef<RenderPipeline>(specification);
	}

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
				case Topology::PatchList: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			}

			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		}

		static VkCullModeFlagBits VulkanCullModeFromCullMode(CullMode cullMode)
		{
			switch (cullMode)
			{
				case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
				case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
				case CullMode::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
				case CullMode::None: return VK_CULL_MODE_NONE;
			}

			return VK_CULL_MODE_BACK_BIT;
		}

		static VkColorComponentFlags GetColorComponentsFromFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R32F: return VK_COLOR_COMPONENT_R_BIT;
				case ImageFormat::R32SI: return VK_COLOR_COMPONENT_R_BIT;
				case ImageFormat::R32UI: return VK_COLOR_COMPONENT_R_BIT;
				case ImageFormat::RGB: return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
				case ImageFormat::RGBA: return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				case ImageFormat::RGBA16F: return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				case ImageFormat::RGBA32F: return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				case ImageFormat::RG16F: return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT;
				case ImageFormat::RG32F: return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT;
				case ImageFormat::SRGB: return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
				default:
					break;
			}

			return VK_COLOR_COMPONENT_R_BIT;
		}
	}

	RenderPipeline::RenderPipeline(const RenderPipelineSpecification& specification)
		: m_specification(specification)
	{
		SetLayout(specification.vertexLayout);
	}

	RenderPipeline::~RenderPipeline()
	{
		auto device = VulkanContext::GetCurrentDevice();

		vkDeviceWaitIdle(device->GetHandle());

		vkDestroyPipelineLayout(device->GetHandle(), m_layout, nullptr);
		vkDestroyPipeline(device->GetHandle(), m_pipeline, nullptr);
	}

	void RenderPipeline::Bind(Ref<CommandBuffer> commandBuffer) const
	{
		auto vulkanCommandBuffer = static_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer());

		VkExtent2D extent{};
		extent.width = m_specification.framebuffer->GetSpecification().width;
		extent.height = m_specification.framebuffer->GetSpecification().height;

		VkViewport viewport{};
		viewport.x = 0.f;
		viewport.y = extent.height;

		viewport.width = extent.width;
		viewport.height = extent.height;
		viewport.height = -viewport.height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor = { { 0, 0 }, extent };
		vkCmdSetViewport(vulkanCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(vulkanCommandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(vulkanCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
	}

	void RenderPipeline::SetLayout(BufferLayout layout)
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

	void RenderPipeline::SetTexture(Ref<Texture2D> texture, uint32_t binding, uint32_t set, uint32_t index)
	{
		auto shader = m_specification.shader;

		LP_CORE_ASSERT(index < m_descriptorSets.size(), "Index must be less than the descriptor set map size!");

		auto& shaderDescriptorSets = shader->GetDescriptorSets();
		auto& imageSamplers = shaderDescriptorSets[set].imageSamplers;
		auto descriptorWrite = shaderDescriptorSets[set].writeDescriptorSets.at(imageSamplers.at(binding).name);
		descriptorWrite.dstSet = m_descriptorSets[index][set];
		descriptorWrite.pImageInfo = &texture->GetDescriptorInfo();

		auto device = VulkanContext::GetCurrentDevice();
		vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
	}

	void RenderPipeline::SetTexture(Ref<Image2D> image, uint32_t set, uint32_t binding, uint32_t index)
	{
		auto shader = m_specification.shader;
		auto vulkanImage = std::reinterpret_pointer_cast<Image2D>(image);

		//TODO: transition to right layout

		auto& shaderDescriptorSets = shader->GetDescriptorSets();
		auto& imageSamplers = shaderDescriptorSets[set].imageSamplers;
		auto descriptorWrite = shaderDescriptorSets[set].writeDescriptorSets.at(imageSamplers.at(binding).name);
		descriptorWrite.dstSet = m_descriptorSets[index][set];
		descriptorWrite.pImageInfo = &vulkanImage->GetDescriptorInfo();

		auto device = VulkanContext::GetCurrentDevice();
		vkUpdateDescriptorSets(device->GetHandle(), 1, &descriptorWrite, 0, nullptr);
	}

	void RenderPipeline::SetPushConstantData(Ref<CommandBuffer> commandBuffer, uint32_t index, const void* data)
	{
		auto shader = m_specification.shader;
		const auto& pushConstants = shader->GetPushConstantRanges();

		if (!pushConstants.empty() && index < pushConstants.size())
		{
			auto vulkanCommandBuffer = reinterpret_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer());
			vkCmdPushConstants(vulkanCommandBuffer, m_layout, pushConstants[index].shaderStage, pushConstants[index].offset, pushConstants[index].size, data);
		}
	}

	void RenderPipeline::CreateDescriptorSets()
	{
		auto swapchain = Application::Get().GetWindow().GetSwapchain();
		auto device = VulkanContext::GetCurrentDevice();

		auto vulkanShader = m_specification.shader;

		auto allDescriptorLayouts = vulkanShader->GetAllDescriptorSetLayouts();

		const uint32_t framesInFlight = m_specification.isSwapchain ? swapchain->GetImageCount() : 1;

		//for (uint32_t i = 0; i < framesInFlight; i++)
		//{
		//	VkDescriptorSetAllocateInfo allocInfo{};
		//	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		//	allocInfo.descriptorPool = static_cast<VkDescriptorPool>(std::reinterpret_pointer_cast<VulkanRenderer>(Renderer::GetRenderer())->GetDescriptorPool());
		//	allocInfo.descriptorSetCount = static_cast<uint32_t>(allDescriptorLayouts.size());
		//	allocInfo.pSetLayouts = allDescriptorLayouts.data();

		//	m_descriptorSets[i].resize(allDescriptorLayouts.size());

		//	LP_VK_CHECK(vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, m_descriptorSets[i].data()));
		//}
	}

	void RenderPipeline::SetupUniformBuffers()
	{
		auto device = VulkanContext::GetCurrentDevice();
		auto vulkanShader = m_specification.shader;

		const uint32_t count = m_specification.isSwapchain ? Renderer::Get().GetCapabilities().framesInFlight : 1;
		auto& shaderDescriptorSets = vulkanShader->GetDescriptorSets();

		for (uint32_t frame = 0; frame < count; frame++)
		{
			for (uint32_t set = 0; set < shaderDescriptorSets.size(); set++)
			{
				auto& uniformBuffers = shaderDescriptorSets[set].uniformBuffers;

				for (const auto& uniformBuffer : uniformBuffers)
				{
					auto writeDescriptor = shaderDescriptorSets[set].writeDescriptorSets.at(uniformBuffer.second->name);
					writeDescriptor.dstSet = m_descriptorSets[frame][set];

					auto vulkanUniformBuffer = m_specification.uniformBufferSets->Get(uniformBuffer.second->bindPoint, set, frame);
					writeDescriptor.pBufferInfo = &vulkanUniformBuffer->GetDescriptorInfo();

					vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);
				}
			}
		}
	}

	void RenderPipeline::Invalidate()
	{
		auto device = VulkanContext::GetCurrentDevice();
		auto swapchain = Application::Get().GetWindow().GetSwapchain();
		auto shader = m_specification.shader;
		Ref<Framebuffer> framebuffer = nullptr;

		if (m_specification.framebuffer)
		{
			framebuffer = std::reinterpret_pointer_cast<Framebuffer>(m_specification.framebuffer);
		}

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
		rasterizer.cullMode = Utils::VulkanCullModeFromCullMode(m_specification.cullMode);
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineTessellationStateCreateInfo tessellation{};
		tessellation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessellation.patchControlPoints = m_specification.tessellationControlPoints;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		std::vector<VkPipelineColorBlendAttachmentState> blendAttachments;

		for (const auto& attachment : m_specification.framebuffer->GetSpecification().attachments.Attachments)
		{
			if (Utils::IsDepthFormat(attachment.textureFormat))
			{
				continue;
			}

			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;

			blendAttachments.emplace_back(colorBlendAttachment);
		}

		colorBlending.attachmentCount = blendAttachments.size();
		colorBlending.pAttachments = blendAttachments.data();

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = m_specification.depthTest ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = m_specification.depthWrite ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

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

		LP_VK_CHECK(vkCreatePipelineLayout(device->GetHandle(), &pipelineLayoutInfo, nullptr, &m_layout));

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
		pipelineInfo.pTessellationState = m_specification.useTessellation ? &tessellation : nullptr;

		LP_VK_CHECK(vkCreateGraphicsPipelines(device->GetHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline));
	}

	void RenderPipeline::BindDescriptorSets(Ref<CommandBuffer> commandBuffer, const std::vector<VkDescriptorSet>& descriptorSets, uint32_t startSet) const
	{
		auto vulkanCommanBuffer = reinterpret_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer());
		vkCmdBindDescriptorSets(vulkanCommanBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, startSet, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
	}

	void RenderPipeline::BindDescriptorSet(Ref<CommandBuffer> commandBuffer, VkDescriptorSet descriptorSet, uint32_t set) const
	{
		auto vulkanCommandBuffer = reinterpret_cast<VkCommandBuffer>(commandBuffer->GetCurrentCommandBuffer());
		vkCmdBindDescriptorSets(vulkanCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, set, 1, &descriptorSet, 0, nullptr);
	}
}