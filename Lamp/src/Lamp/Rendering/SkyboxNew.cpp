#include "lppch.h"
#include "SkyboxNew.h"

#include "Lamp/Core/Time/ScopedTimer.h"

#include "Lamp/Rendering/Buffers/Framebuffer.h"
#include "Lamp/Rendering/RenderPipeline.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"

#include "Platform/Vulkan/VulkanAllocator.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanUtility.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanBuffer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Lamp
{
	SkyboxNew::SkyboxNew()
	{
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

			m_cubeMesh = CreateRef<SubMesh>(positions, indices, 0);
		}
		//////////////

		//GenerateBRDF();
		GenerateIrradianceCube();
		//GeneratePrefilterCube();
	}

	SkyboxNew::~SkyboxNew()
	{
		auto device = VulkanContext::GetCurrentDevice();
		VulkanAllocator allocator;

		//vkDestroySampler(device->GetHandle(), m_irradianceSampler, nullptr);
		//vkDestroyImageView(device->GetHandle(), m_irradianceImageView, nullptr);
		//
		//allocator.DestroyImage(m_irradianceImage, m_irradianceAlloc);
	}

	void SkyboxNew::GenerateBRDF()
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
		m_brdfFramebuffer = pipelineSpec.framebuffer;

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

	void SkyboxNew::GenerateIrradianceCube()
	{
		auto device = VulkanContext::GetCurrentDevice();

		const VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;
		const uint32_t dimension = 64;
		const uint32_t numMips = static_cast<uint32_t>(std::floor(std::log2(dimension))) + 1;

		//Image
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format;
		imageInfo.extent.width = dimension;
		imageInfo.extent.height = dimension;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = numMips;
		imageInfo.arrayLayers = 6;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VulkanAllocator allocator;
		m_irradianceAlloc = allocator.AllocateImage(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_irradianceImage);

		//Image view
		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		imageViewInfo.format = format;
		imageViewInfo.image = m_irradianceImage;

		imageViewInfo.subresourceRange = {};
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.levelCount = numMips;
		imageViewInfo.subresourceRange.layerCount = 6;

		VkResult result = vkCreateImageView(device->GetHandle(), &imageViewInfo, nullptr, &m_irradianceImageView);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create image view!");

		//Sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

		samplerInfo.minLod = 0.f;
		samplerInfo.maxLod = static_cast<float>(numMips);

		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		result = vkCreateSampler(device->GetHandle(), &samplerInfo, nullptr, &m_irradianceSampler);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create sampler!");

		m_irradianceDescriptor.imageView = m_irradianceImageView;
		m_irradianceDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_irradianceDescriptor.sampler = m_irradianceSampler;

		// FB, Att, RP, Pipe, etc.
		VkAttachmentDescription attachmentDesc{};
		attachmentDesc.format = format;
		attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDesc{};
		subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.colorAttachmentCount = 1;
		subpassDesc.pColorAttachments = &colorReference;

		//Subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		//Renderpass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &attachmentDesc;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDesc;
		renderPassInfo.dependencyCount = 2;
		renderPassInfo.pDependencies = dependencies.data();

		VkRenderPass renderPass;
		result = vkCreateRenderPass(device->GetHandle(), &renderPassInfo, nullptr, &renderPass);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create render pass");

		struct
		{
			VkImage image;
			VkImageView view;
			VmaAllocation allocation;
			VkFramebuffer framebuffer;

		} offscreenFramebuffer;

		//Offscreen framebuffer
		{
			VkImageCreateInfo imageCreateInfo{};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = format;
			imageCreateInfo.extent.width = dimension;
			imageCreateInfo.extent.height = dimension;
			imageCreateInfo.extent.depth = 1;
			imageCreateInfo.mipLevels = 1;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			offscreenFramebuffer.allocation = allocator.AllocateImage(imageCreateInfo, VMA_MEMORY_USAGE_GPU_ONLY, offscreenFramebuffer.image);

			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = format;
			viewInfo.flags = 0;
			viewInfo.subresourceRange = {};
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.image = offscreenFramebuffer.image;

			result = vkCreateImageView(device->GetHandle(), &viewInfo, nullptr, &offscreenFramebuffer.view);
			LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create view!");

			VkFramebufferCreateInfo bufferInfo{};
			bufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			bufferInfo.renderPass = renderPass;
			bufferInfo.attachmentCount = 1;
			bufferInfo.pAttachments = &offscreenFramebuffer.view;
			bufferInfo.width = dimension;
			bufferInfo.height = dimension;
			bufferInfo.layers = 1;

			result = vkCreateFramebuffer(device->GetHandle(), &bufferInfo, nullptr, &offscreenFramebuffer.framebuffer);

			Utility::TransitionImageLayout(offscreenFramebuffer.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}

		//Descriptors
		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

		auto& binding = layoutBindings.emplace_back();
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding.descriptorCount = 1;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		vkCreateDescriptorSetLayout(device->GetHandle(), &layoutInfo, nullptr, &descriptorSetLayout);

		//Descriptor pool
		std::vector<VkDescriptorPoolSize> poolSizes;

		auto& size = poolSizes.emplace_back();
		size.descriptorCount = 1;
		size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.maxSets = 1;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = poolSizes.data();

		VkDescriptorPool descriptorPool;
		vkCreateDescriptorPool(device->GetHandle(), &poolInfo, nullptr, &descriptorPool);

		VkDescriptorSet descriptorSet;
		VkDescriptorSetAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		allocInfo.descriptorSetCount = 1;
		
		vkAllocateDescriptorSets(device->GetHandle(), &allocInfo, &descriptorSet);

		VkWriteDescriptorSet writeDescriptor{};
		writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptor.dstSet = descriptorSet;
		writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptor.dstArrayElement = 0;
		writeDescriptor.pImageInfo = &m_cubeDescriptor;
		vkUpdateDescriptorSets(device->GetHandle(), 1, &writeDescriptor, 0, nullptr);

		// Pipeline layout
		struct PushBlock
		{
			glm::mat4 mvp;
			// Sampling deltas
			float deltaPhi = (2.0f * glm::pi<float>()) / 180.0f;
			float deltaTheta = (0.5f * glm::pi<float>()) / 64.0f;
		} pushBlock;

		VkPipelineLayout pipelinelayout;
		std::vector<VkPushConstantRange> pushConstantRanges;
		
		auto& pushConstant = pushConstantRanges.emplace_back();
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstant.size = sizeof(PushBlock);
		pushConstant.offset = 0;

		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutCI.pushConstantRangeCount = 1;
		pipelineLayoutCI.pPushConstantRanges = pushConstantRanges.data();
		vkCreatePipelineLayout(device->GetHandle(), &pipelineLayoutCI, nullptr, &pipelinelayout);

		// Pipeline
		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(Vertex);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputAttributeDescription> attrDescs;
		
		auto& pos = attrDescs.emplace_back();
		pos.binding = 0;
		pos.location = 0;
		pos.offset = 0;
		pos.format = VK_FORMAT_R32G32B32_SFLOAT;

		auto& normal = attrDescs.emplace_back();
		normal.binding = 0;
		normal.location = 1;
		normal.offset = sizeof(float) * 3;
		normal.format = VK_FORMAT_R32G32B32_SFLOAT;

		auto& tangent = attrDescs.emplace_back();
		normal.binding = 0;
		normal.location = 2;
		normal.offset = sizeof(float) * 6;
		normal.format = VK_FORMAT_R32G32B32_SFLOAT;

		auto& bitangent = attrDescs.emplace_back();
		normal.binding = 0;
		normal.location = 3;
		normal.offset = sizeof(float) * 9;
		normal.format = VK_FORMAT_R32G32B32_SFLOAT;

		auto& texCoords = attrDescs.emplace_back();
		normal.binding = 0;
		normal.location = 4;
		normal.offset = sizeof(float) * 12;
		normal.format = VK_FORMAT_R32G32_SFLOAT;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = attrDescs.empty() ? 0 : 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDescs.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
		vertexInputInfo.pVertexAttributeDescriptions = attrDescs.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyState.primitiveRestartEnable = VK_FALSE;
		inputAssemblyState.flags = 0;
		
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendState.logicOpEnable = VK_FALSE;
		colorBlendState.logicOp = VK_LOGIC_OP_COPY;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &blendAttachmentState;
		colorBlendState.blendConstants[0] = 0.0f;
		colorBlendState.blendConstants[1] = 0.0f;
		colorBlendState.blendConstants[2] = 0.0f;
		colorBlendState.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.stencilTestEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;

		VkPipelineMultisampleStateCreateInfo multisampleState{};
		multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleState.sampleShadingEnable = VK_FALSE;
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;

		auto shader = std::reinterpret_pointer_cast<VulkanShader>(ShaderLibrary::GetShader("irradianceCube"));

		VkGraphicsPipelineCreateInfo pipelineCI{};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCI.pInputAssemblyState = &inputAssemblyState;
		pipelineCI.pRasterizationState = &rasterizer;
		pipelineCI.pColorBlendState = &colorBlendState;
		pipelineCI.pMultisampleState = &multisampleState;
		pipelineCI.pViewportState = &viewportState;
		pipelineCI.pDepthStencilState = &depthStencilState;
		pipelineCI.pDynamicState = &dynamicState;
		pipelineCI.stageCount = 2;
		pipelineCI.pStages = shader->GetShaderStageInfos().data();
		pipelineCI.renderPass = renderPass;
		pipelineCI.pVertexInputState = &vertexInputInfo;

		VkPipeline pipeline;
		vkCreateGraphicsPipelines(device->GetHandle(), nullptr, 1, &pipelineCI, nullptr, &pipeline);

		// Render

		VkClearValue clearValues[1];
		clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		// Reuse render pass from example pass
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = offscreenFramebuffer.framebuffer;
		renderPassBeginInfo.renderArea.extent.width = dimension;
		renderPassBeginInfo.renderArea.extent.height = dimension;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;

		std::vector<glm::mat4> matrices = 
		{
			// POSITIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_X
			glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Y
			glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// POSITIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
			// NEGATIVE_Z
			glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		};

		VkCommandBuffer cmdBuf = device->GetCommandBuffer(true);

		VkViewport viewport{};
		viewport.height = (float)dimension;
		viewport.width = (float)dimension;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		viewport.x = 0;
		viewport.y = 0;

		VkRect2D scissor{};
		scissor.extent.height = dimension;
		scissor.extent.width = dimension;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = numMips;
		subresourceRange.layerCount = 6;

		// Change image layout for all cubemap faces to transfer destination
		Utility::TransitionImageLayout(m_irradianceImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

		for (uint32_t m = 0; m < numMips; m++)
		{
			for (uint32_t f = 0; f < 6; f++)
			{
				viewport.width = static_cast<float>(dimension * std::pow(0.5f, m));
				viewport.height = static_cast<float>(dimension * std::pow(0.5f, m));
				vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

				// Render scene from cube face's point of view
				vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				// Update shader push constant block
				pushBlock.mvp = glm::perspective(glm::pi<float>() / 2.0f, 1.0f, 0.1f, 512.0f) * matrices[f];

				vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlock), &pushBlock);
				//
				vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
				vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1, &descriptorSet, 0, NULL);

				Renderer::SubmitCube();

				auto vulkanVertBuff = std::reinterpret_pointer_cast<VulkanVertexBuffer>(m_cubeMesh->GetVertexArray()->GetVertexBuffers()[0]);
				auto vulkanIndexBuff = std::reinterpret_pointer_cast<VulkanIndexBuffer>(m_cubeMesh->GetVertexArray()->GetIndexBuffer());

				VkDeviceSize offsets[] = { 0 };
				VkBuffer buffers[] = { vulkanVertBuff->GetBuffer() };

				vkCmdBindVertexBuffers(cmdBuf, 0, 1, buffers, offsets);
				vkCmdBindIndexBuffer(cmdBuf, vulkanIndexBuff->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

				vkCmdDrawIndexed(cmdBuf, vulkanIndexBuff->GetCount(), 1, 0, 0, 0);

				vkCmdEndRenderPass(cmdBuf);

				Utility::TransitionImageLayout(offscreenFramebuffer.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

				// Copy region for transfer from framebuffer to cube face
				VkImageCopy copyRegion = {};

				copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.srcSubresource.baseArrayLayer = 0;
				copyRegion.srcSubresource.mipLevel = 0;
				copyRegion.srcSubresource.layerCount = 1;
				copyRegion.srcOffset = { 0, 0, 0 };

				copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.dstSubresource.baseArrayLayer = f;
				copyRegion.dstSubresource.mipLevel = m;
				copyRegion.dstSubresource.layerCount = 1;
				copyRegion.dstOffset = { 0, 0, 0 };

				copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
				copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
				copyRegion.extent.depth = 1;

				vkCmdCopyImage(
					cmdBuf,
					offscreenFramebuffer.image,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					m_irradianceImage,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					1,
					&copyRegion);

				Utility::TransitionImageLayout(m_irradianceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			}
		}

		device->FlushCommandBuffer(cmdBuf);

		Utility::TransitionImageLayout(m_irradianceImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyRenderPass(device->GetHandle(), renderPass, nullptr);
		vkDestroyFramebuffer(device->GetHandle(), offscreenFramebuffer.framebuffer, nullptr);
		vkDestroyImageView(device->GetHandle(), offscreenFramebuffer.view, nullptr);
		allocator.DestroyImage(offscreenFramebuffer.image, offscreenFramebuffer.allocation);
		vkDestroyDescriptorPool(device->GetHandle(), descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device->GetHandle(), descriptorSetLayout, nullptr);
		vkDestroyPipeline(device->GetHandle(), pipeline, nullptr);
		vkDestroyPipelineLayout(device->GetHandle(), pipelinelayout, nullptr);
	}

	void SkyboxNew::GeneratePrefilterCube()
	{

	}
}