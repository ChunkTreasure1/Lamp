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
		uint32_t GetCurrentCommandBufferIndex() override;

	private:
		std::vector<VkCommandPool> m_commandPools;
		std::vector<VkCommandBuffer> m_commandBuffers;

		bool m_swapchainTarget;
		uint32_t m_currentCommandPool;
		uint32_t m_count;
	};
}