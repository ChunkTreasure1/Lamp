#include "lppch.h"
#include "CommandBuffer.h"

#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace Lamp
{
	Ref<CommandBuffer> CommandBuffer::Create(uint32_t count, bool swapchainTarget)
	{
		return CreateRef<VulkanCommandBuffer>(count, swapchainTarget);
	}
}