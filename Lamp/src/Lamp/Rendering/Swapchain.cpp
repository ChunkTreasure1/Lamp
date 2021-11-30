#include "lppch.h"
#include "Swapchain.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanSwapchain.h"

namespace Lamp
{
	Scope<Swapchain> Swapchain::Create(void* instance, void* device)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_CRITICAL("None is not supported!"); return nullptr;
			case RendererAPI::API::Vulkan: return CreateScope<VulkanSwapchain>(instance, device);
		}

		return nullptr;
	}
}