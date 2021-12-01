#pragma once

#include "Lamp/Rendering/CommandBuffer.h"

#include <vulkan/vulkan_core.h>

namespace Lamp
{
	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		VulkanCommandBuffer(Ref<RenderPipeline> renderPipeline);
		~VulkanCommandBuffer();

		void Begin() override;
		void End() override;

		void* GetCurrentCommandBuffer() override;

	private:
		std::vector<VkCommandBuffer> m_commandBuffers;
	};
}