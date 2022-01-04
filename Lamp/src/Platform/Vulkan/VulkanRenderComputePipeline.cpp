#include "lppch.h"
#include "VulkanRenderComputePipeline.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

#include "Lamp/Core/Application.h"
#include "Lamp/Rendering/Swapchain.h"

namespace Lamp
{
	static VkFence s_computeFence = nullptr;

	VulkanRenderComputePipeline::VulkanRenderComputePipeline(Ref<Shader> computeShader)
		: m_shader(computeShader)
	{
		CreatePipeline();
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
			m_activeComputeCommandBuffer = VulkanContext::GetCurrentDevice()->GetCommandBuffer(true, true);
			m_usingGraphicsQueue = false;
		}

		vkCmdBindPipeline(m_activeComputeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
	}

	void VulkanRenderComputePipeline::End()
	{
		LP_CORE_ASSERT(m_activeComputeCommandBuffer, "Active compute command buffer cannot be null!");

		VkDevice device = VulkanContext::GetCurrentDevice()->GetHandle();
		if (!m_usingGraphicsQueue)
		{
			VkQueue computeQueue = VulkanContext::GetCurrentDevice()->GetComputeQueue();

			vkEndCommandBuffer(m_activeComputeCommandBuffer);

			if (!s_computeFence)
			{
				VkFenceCreateInfo fenceCreateInfo{};
				fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
				VkResult result = vkCreateFence(device, &fenceCreateInfo, nullptr, &s_computeFence);
				LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create fence!");
			}

			vkWaitForFences(device, 1, &s_computeFence, VK_TRUE, UINT64_MAX);
			vkResetFences(device, 1, &s_computeFence);

			VkSubmitInfo computeSubmitInfo{};
			computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			computeSubmitInfo.commandBufferCount = 1;
			computeSubmitInfo.pCommandBuffers = &m_activeComputeCommandBuffer;
			
			VkResult result = vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, s_computeFence);
			vkWaitForFences(device, 1, &s_computeFence, VK_TRUE, UINT64_MAX);
		}

		m_activeComputeCommandBuffer = nullptr;
	}

	void VulkanRenderComputePipeline::Execute(VkDescriptorSet* descriptorSets, uint32_t descriptorSetCount, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{
		VkDevice device = VulkanContext::GetCurrentDevice()->GetHandle();
		VkQueue computeQueue = VulkanContext::GetCurrentDevice()->GetComputeQueue();

		VkCommandBuffer computeCommandBuffer = VulkanContext::GetCurrentDevice()->GetCommandBuffer(true, true);

		vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
		for (uint32_t i = 0; i < descriptorSetCount; i++)
		{
			vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipelineLayout, 0, 1, &descriptorSets[i], 0, 0);
			vkCmdDispatch(computeCommandBuffer, groupCountX, groupCountY, groupCountZ);
		}

		vkEndCommandBuffer(computeCommandBuffer);
		if (!s_computeFence)
		{
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			VkResult result = vkCreateFence(device, &fenceInfo, nullptr, &s_computeFence);
		}

		vkWaitForFences(device, 1, &s_computeFence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &s_computeFence);

		VkSubmitInfo computeSubmitInfo{};
		computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		computeSubmitInfo.commandBufferCount = 1;
		computeSubmitInfo.pCommandBuffers = &computeCommandBuffer;
		VkResult result = vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, s_computeFence);

		vkWaitForFences(device, 1, &s_computeFence, VK_TRUE, UINT64_MAX);
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

		VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_computePipelineLayout);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create pipeline layout!");

		VkComputePipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = m_computePipelineLayout;
		pipelineCreateInfo.flags = 0;
		
		const auto& shaderStages = vulkanShader->GetShaderStageInfos();
		pipelineCreateInfo.stage = shaderStages[0];

		VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		result = vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &m_computePipelineCache);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create pipeline cache!");

		result = vkCreateComputePipelines(device, m_computePipelineCache, 1, &pipelineCreateInfo, nullptr, &m_computePipeline);
	}
}