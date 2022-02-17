#pragma once

#include "Lamp/Core/Core.h"

#include <vulkan/vulkan_core.h>

namespace Lamp
{
	class CommandBuffer
	{
	public:
		CommandBuffer(uint32_t count, bool swapchainTarget);
		~CommandBuffer();

		void Begin();
		void End(bool compute = false);
 		
		VkCommandBuffer GetCurrentCommandBuffer();
		uint32_t GetCurrentCommandBufferIndex();

		VkCommandBuffer GetCommandBuffer(uint32_t index);

		static Ref<CommandBuffer> Create(uint32_t count, bool swapchainTarget = false);

	private:
		std::vector<VkCommandPool> m_commandPools;
		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<VkFence> m_fences;

		bool m_swapchainTarget;
		uint32_t m_currentCommandPool;
		uint32_t m_count;
	};
}