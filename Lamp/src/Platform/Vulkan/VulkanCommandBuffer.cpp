#include "lppch.h"
#include "VulkanCommandBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanUtility.h"

#include "Lamp/Core/Application.h"

namespace Lamp
{
	VulkanCommandBuffer::VulkanCommandBuffer(uint32_t count, bool swapchainTarget)
		: m_swapchainTarget(swapchainTarget), m_currentCommandPool(0)
	{
		auto swapchain = std::reinterpret_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());
		auto device = VulkanContext::GetCurrentDevice();

		if (!swapchainTarget)
		{
			m_count = count;
			auto families = Utility::FindQueueFamilies(device->GetPhysicalDevice()->GetHandle(), swapchain->GetSurface());

			m_commandPools.resize(count);

			for (uint32_t i = 0; i < count; i++)
			{
				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.queueFamilyIndex = families.graphicsFamily.value();
				poolInfo.flags = 0;

				VkResult result = vkCreateCommandPool(device->GetHandle(), &poolInfo, nullptr, &m_commandPools[i]);
				LP_CORE_ASSERT(result == VK_SUCCESS, "VulkanCommandBuffer: Unable to create command pool!");
			}

			m_commandBuffers.resize(count);

			for (uint32_t i = 0; i < count; i++)
			{
				VkCommandBufferAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = m_commandPools[i];
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandBufferCount = 1;

				VkResult result = vkAllocateCommandBuffers(device->GetHandle(), &allocInfo, &m_commandBuffers[i]);
				LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to allocate command buffers!");
			}

			//TODO: create fences for command buffers
		}
		else
		{
			m_count = Renderer::GetCapabilities().framesInFlight;
			
			m_commandPools.resize(m_count);
			m_commandBuffers.resize(m_count);
			for (uint32_t frame = 0; frame < m_count; frame++)
			{
				m_commandBuffers[frame] = swapchain->GetDrawCommandBuffer(frame);
				m_commandPools[frame] = swapchain->GetDrawCommandPool(frame);
			}
		}
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		if (!m_swapchainTarget) // only remove if we own the command buffers
		{
			auto device = VulkanContext::GetCurrentDevice();
			vkDeviceWaitIdle(device->GetHandle());

			for (uint32_t i = 0; i < m_commandPools.size(); i++)
			{
				vkDestroyCommandPool(device->GetHandle(), m_commandPools[i], nullptr);
			}

			m_commandPools.clear();
			m_commandBuffers.clear();
		}
	}

	void VulkanCommandBuffer::Begin()
	{
		auto device = VulkanContext::GetCurrentDevice();
		uint32_t frame = m_swapchainTarget ? Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame() : m_currentCommandPool;

		VkResult result;
		if (!m_swapchainTarget)
		{
			result = vkResetCommandPool(device->GetHandle(), m_commandPools[frame], 0);
			LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to reset command pool!");
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	
		VkCommandBuffer commandBuffer = m_commandBuffers[frame];
		result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to begin command buffer!");
	}

	void VulkanCommandBuffer::End()
	{
		uint32_t frame = m_swapchainTarget ? Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame() : m_currentCommandPool;
		auto device = VulkanContext::GetCurrentDevice();

		VkResult result = vkEndCommandBuffer(m_commandBuffers[frame]);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to end command buffer!");

		if (!m_swapchainTarget)
		{
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_commandBuffers[frame];

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = 0;

			VkFence fence;
			result = vkCreateFence(device->GetHandle(), &fenceInfo, nullptr, &fence);
			LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create fence!");

			result = vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, fence);
			LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to submit to queue!");

			result = vkWaitForFences(device->GetHandle(), 1, &fence, VK_TRUE, UINT64_MAX);
			LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to wait for fences!");

			vkDestroyFence(device->GetHandle(), fence, nullptr);
		}

		m_currentCommandPool = m_swapchainTarget ? Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame() : ((m_currentCommandPool + 1) % m_count);
	}

	void* VulkanCommandBuffer::GetCurrentCommandBuffer()
	{
		uint32_t frame = m_swapchainTarget ? Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame() : m_currentCommandPool;
		return m_commandBuffers[frame];
	}

	uint32_t VulkanCommandBuffer::GetCurrentCommandBufferIndex()
	{
		return m_swapchainTarget ? Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame() : m_currentCommandPool;
	}

	VkCommandBuffer VulkanCommandBuffer::GetCommandBuffer(uint32_t index)
	{
		return m_commandBuffers[index];
	}
}