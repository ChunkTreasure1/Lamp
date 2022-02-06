#include "lppch.h"
#include "Texture2D.h"

#include "Platform/Vulkan/VulkanTexture2D.h"

namespace Lamp
{
	Ref<Texture2D> Texture2D::Create(ImageFormat format, uint32_t width, uint32_t height)
	{
		return CreateRef<VulkanTexture2D>(format, width, height);
	}

	Ref<Texture2D> Texture2D::Create(const std::filesystem::path& path, bool generateMips)
	{
		return CreateRef<VulkanTexture2D>(path, generateMips);
	}

	Ref<Texture2D> Texture2D::Create()
	{
		return CreateRef<VulkanTexture2D>();
	}
}