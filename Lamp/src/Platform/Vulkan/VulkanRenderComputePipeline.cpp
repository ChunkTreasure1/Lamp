#include "lppch.h"
#include "VulkanRenderComputePipeline.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanUtility.h"
#include "Platform/Vulkan/VulkanRenderer.h"

#include "Lamp/Core/Application.h"
#include "Lamp/Rendering/Swapchain.h"

namespace Lamp
{
	VulkanRenderComputePipeline::VulkanRenderComputePipeline(Ref<Shader> computeShader)
		: m_shader(computeShader)
	{
		CreatePipeline();
	}

	VulkanRenderComputePipeline::~VulkanRenderComputePipeline()
	{
		auto device = VulkanContext::GetCurrentDevice();
	}

	void VulkanRenderComputePipeline::Begin(Ref<CommandBuffer> commandBuffer)
	{
		LP_CORE_ASSERT(!m_activeComputeCommandBuffer, "Active compute command buffer has to be null!");

		if (commandBuffer)
		{
			uint32_t frameIndex = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();
			m_activeComputeCommandBuffer = std::reinterpret_pointer_cast<VulkanCommandBuffer>(commandBuffer)->GetCommandBuffer(frameIndex);
			m_usingGraphicsQueue = true;
		}
		else
		{
			uint32_t currentFrame = Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame();
			auto device = VulkanContext::GetCurrentDevice();

			m_commandBuffer->Begin();
			m_usingGraphicsQueue = false;
			m_activeComputeCommandBuffer = static_cast<VkCommandBuffer>(m_commandBuffer->GetCurrentCommandBuffer());
		}

		vkCmdBindPipeline(m_activeComputeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
	}

	void VulkanRenderComputePipeline::End()
	{
		LP_CORE_ASSERT(m_activeComputeCommandBuffer, "Active compute command buffer cannot be null!");

		VkDevice device = VulkanContext::GetCurrentDevice()->GetHandle();
		if (!m_usingGraphicsQueue)
		{
			m_commandBuffer->End();
		}

		m_activeComputeCommandBuffer = nullptr;
	}

	void VulkanRenderComputePipeline::Execute(VkDescriptorSet* descriptorSets, uint32_t descriptorSetCount, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		LP_PROFILE_FUNCTION();

		VkDevice device = VulkanContext::GetCurrentDevice()->GetHandle();
		VkQueue computeQueue = VulkanContext::GetCurrentDevice()->GetComputeQueue();

		m_commandBuffer->Begin();
		VkCommandBuffer computeCommandBuffer = static_cast<VkCommandBuffer>(m_commandBuffer->GetCurrentCommandBuffer());

		vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
		for (uint32_t i = 0; i < descriptorSetCount; i++)
		{
			vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipelineLayout, 0, 1, &descriptorSets[i], 0, 0);
			vkCmdDispatch(computeCommandBuffer, groupCountX, groupCountY, groupCountZ);
		}

		m_commandBuffer->End(true);
	}

	void VulkanRenderComputePipeline::Dispatch(VkDescriptorSet descriptorSet, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		LP_CORE_ASSERT(m_activeComputeCommandBuffer, "Active command buffer cannot be null!");

		vkCmdBindDescriptorSets(m_activeComputeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipelineLayout, 0, 1, &descriptorSet, 0, 0);
		vkCmdDispatch(m_activeComputeCommandBuffer, groupCountX, groupCountY, groupCountZ);
	}

	void VulkanRenderComputePipeline::SetPushConstants(const void* data, uint32_t size)
	{
		vkCmdPushConstants(m_activeComputeCommandBuffer, m_computePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, size, data);
	}

	void VulkanRenderComputePipeline::CreatePipeline()
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetHandle();

		auto vulkanShader = std::reinterpret_pointer_cast<VulkanShader>(m_shader);
		auto descriptorSetLayouts = vulkanShader->GetAllDescriptorSetLayouts();

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	
		const auto& pushConstantRanges = vulkanShader->GetAllPushConstantRanges();
		if (!pushConstantRanges.empty())
		{
			pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.size();
			pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
		}

		LP_VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_computePipelineLayout));

		VkComputePipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = m_computePipelineLayout;
		pipelineCreateInfo.flags = 0;
		
		const auto& shaderStages = vulkanShader->GetShaderStageInfos();
		pipelineCreateInfo.stage = shaderStages[0];

		VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		LP_VK_CHECK(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &m_computePipelineCache));
		LP_VK_CHECK(vkCreateComputePipelines(device, m_computePipelineCache, 1, &pipelineCreateInfo, nullptr, &m_computePipeline));
	
		uint32_t framesInFlight = Renderer::Get().GetCapabilities().framesInFlight;
		m_commandBuffer = CommandBuffer::Create(framesInFlight, false);
	}
}