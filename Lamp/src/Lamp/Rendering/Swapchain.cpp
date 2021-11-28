#include "lppch.h"
#include "Swapchain.h"

#include "Lamp/Rendering/Renderer.h"

namespace Lamp
{
	Scope<Swapchain> Swapchain::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_CRITICAL("None is not supported!"); return nullptr;
			case RendererAPI::API::Vulkan: return CreateRef<VulkanSwapchain>();
		}

		return nullptr;
	}
}