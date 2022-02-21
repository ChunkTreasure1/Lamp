#include "lppch.h"
#include "Texture2D.h"

#include "Platform/Vulkan/VulkanTexture2D.h"

namespace Lamp
{
	Ref<Texture2D> Texture2D::Create(ImageFormat format, uint32_t width, uint32_t height, bool saveBuffers)
	{
		return CreateRef<VulkanTexture2D>(format, width, height, saveBuffers);
	}

	Ref<Texture2D> Texture2D::Create(const std::filesystem::path& path, bool generateMips)
	{
		return CreateRef<VulkanTexture2D>(path, generateMips);
	}
}