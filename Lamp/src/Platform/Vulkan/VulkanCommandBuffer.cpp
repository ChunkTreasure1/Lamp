#include "lppch.h"
#include "VulkanCommandBuffer.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanUtility.h"

#include "Lamp/Core/Application.h"

namespace Lamp
{
	VulkanCommandBuffer::VulkanCommandBuffer(uint32_t count, bool swapchainTarget)
	{
		auto swapchain = std::reinterpret_pointer_cast<VulkanSwapchain>(Application::Get().GetWindow().GetSwapchain());
		auto device = VulkanContext::GetCurrentDevice();

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

		}
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
	}

	void VulkanCommandBuffer::Begin()
	{

	}

	void VulkanCommandBuffer::End()
	{
	}

	void* VulkanCommandBuffer::GetCurrentCommandBuffer()
	{
	}
}