#include "lppch.h"
#include "TextureCube.h"

#include "Platform/Vulkan/VulkanTextureCube.h"

namespace Lamp
{
	Ref<TextureCube> TextureCube::Create(ImageFormat format, uint32_t width, uint32_t height)
	{
		return CreateRef<VulkanTextureCube>(format, width, height);
	}

	Ref<TextureCube> TextureCube::Create(const std::filesystem::path& path)
	{
		return CreateRef<VulkanTextureCube>(path);
	}
}