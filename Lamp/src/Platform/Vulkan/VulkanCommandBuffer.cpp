#include "lppch.h"
#include "VulkanCommandBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanUtility.h"
#include "Platform/Vulkan/VulkanRenderer.h"

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

				LP_VK_CHECK(vkCreateCommandPool(device->GetHandle(), &poolInfo, nullptr, &m_commandPools[i]));
			}

			m_commandBuffers.resize(count);

			for (uint32_t i = 0; i < count; i++)
			{
				VkCommandBufferAllocateInfo allocInfo{};
				allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool = m_commandPools[i];
				allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandBufferCount = 1;

				LP_VK_CHECK(vkAllocateCommandBuffers(device->GetHandle(), &allocInfo, &m_commandBuffers[i]));
			}

			m_fences.resize(count);
			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			for (uint32_t i = 0; i < count; i++)
			{
				LP_VK_CHECK(vkCreateFence(device->GetHandle(), &fenceInfo, nullptr, &m_fences[i]));
			}
		}
		else
		{
			m_count = Renderer::Get().GetCapabilities().framesInFlight;
			
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

			for (auto& fence : m_fences)
			{
				vkDestroyFence(device->GetHandle(), fence, nullptr);
			}

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

		if (!m_swapchainTarget)
		{
			vkWaitForFences(device->GetHandle(), 1, &m_fences[frame], VK_TRUE, UINT64_MAX);
			LP_VK_CHECK(vkResetCommandPool(device->GetHandle(), m_commandPools[frame], 0));
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	
		VkCommandBuffer commandBuffer = m_commandBuffers[frame];
		LP_VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
	}

	void VulkanCommandBuffer::End(bool compute)
	{
		uint32_t frame = m_swapchainTarget ? Application::Get().GetWindow().GetSwapchain()->GetCurrentFrame() : m_currentCommandPool;
		auto device = VulkanContext::GetCurrentDevice();

		LP_VK_CHECK(vkEndCommandBuffer(m_commandBuffers[frame]));

		if (!m_swapchainTarget)
		{
			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_commandBuffers[frame];

			vkResetFences(device->GetHandle(), 1, &m_fences[frame]);
			LP_VK_CHECK(vkQueueSubmit(compute ? device->GetComputeQueue() : device->GetGraphicsQueue(), 1, &submitInfo, m_fences[frame]));
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