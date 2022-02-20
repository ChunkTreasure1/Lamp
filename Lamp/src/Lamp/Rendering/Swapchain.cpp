#include "lppch.h"
#include "Swapchain.h"

#include "Platform/Vulkan/VulkanSwapchain.h"

namespace Lamp
{
	Scope<Swapchain> Swapchain::Create(void* instance, void* device)
	{
		return CreateScope<VulkanSwapchain>(instance, device);
	}
}