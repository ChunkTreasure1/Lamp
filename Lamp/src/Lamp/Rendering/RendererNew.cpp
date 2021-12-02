#include "lppch.h"
#include "RendererNew.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanRenderer.h"

namespace Lamp
{
	Ref<RendererNew> RendererNew::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None: LP_CORE_ASSERT(false, "No API supported!");
			case RendererAPI::API::Vulkan: return CreateRef<VulkanRenderer>();
		}
	}
}