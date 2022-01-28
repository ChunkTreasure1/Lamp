#include "lppch.h"
#include "GraphicsContext.h"

#include "Platform/Vulkan/VulkanContext.h"

namespace Lamp
{
	Ref<GraphicsContext> GraphicsContext::Create(void* data)
	{
		return CreateRef<VulkanContext>(static_cast<GLFWwindow*>(data));
	}
}