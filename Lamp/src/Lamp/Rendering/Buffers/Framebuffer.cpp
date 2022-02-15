#include "lppch.h"
#include "Framebuffer.h"

#include "Platform/Vulkan/VulkanFramebuffer.h"

namespace Lamp
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		return CreateRef<VulkanFramebuffer>(spec);
	}
}