#include "lppch.h"
#include "CommandBuffer.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace Lamp
{
	Ref<CommandBuffer> CommandBuffer::Create(uint32_t count, bool swapchainTarget)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!") return nullptr;
			case RendererAPI::API::Vulkan: return CreateRef<VulkanCommandBuffer>(count, swapchainTarget);
		}

		return nullptr;
	}
}