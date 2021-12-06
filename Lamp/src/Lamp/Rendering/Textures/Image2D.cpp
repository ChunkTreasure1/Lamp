#include "lppch.h"
#include "Image2D.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanImage2D.h"

namespace Lamp
{
	Ref<Image2D> Image2D::Create(const ImageSpecification& specification, const void* data)
	{
		switch (RendererAPI::GetAPI())
		{
			case RendererAPI::API::Vulkan: return CreateRef<VulkanImage2D>(specification, data);
		}

		return nullptr;
	}
}