
#include "lppch.h"
#include "UIUtility.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanTexture2D.h"

using namespace Lamp;

ImTextureID UI::GetTextureID(Ref<Lamp::Texture2D> texture)
{
	if (Renderer::GetAPI() == RendererAPI::API::Vulkan)
	{
		Ref<VulkanTexture2D> vulkanTexture = std::reinterpret_pointer_cast<VulkanTexture2D>(texture);
		const VkDescriptorImageInfo& imageInfo = vulkanTexture->GetDescriptorInfo();

		if (!imageInfo.imageView)
		{
			return nullptr;
		}
	}
}
