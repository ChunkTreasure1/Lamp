#pragma once

#include "Lamp/Rendering/CommandBuffer.h"

#include <vulkan/vulkan_core.h>

namespace Lamp
{
	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(uint32_t count, bool swapchainTarget);
		~VulkanCommandBuffer();

		void Begin() override;
		void End() override;

		void* GetCurrentCommandBuffer() override;

	private:
		std::vector<VkCommandPool> m_commandPools;
		std::vector<VkCommandBuffer> m_commandBuffers;
	};
}