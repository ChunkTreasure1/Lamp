#include "lppch.h"
#include "VulkanTexture2D.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtility.h"

#include <stb/stb_image.h>

namespace Lamp
{
	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
	{

	}

	VulkanTexture2D::VulkanTexture2D(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			LP_CORE_ERROR("File not found {0}", path.string());
			SetFlag(AssetFlag::Missing);
			return;
		}

		int width;
		int height;
		int channels;
		stbi_set_flip_vertically_on_load(1);
		uint8_t* imageData = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!imageData)
		{
			LP_CORE_ERROR("Unable to load texture {0}", path.string());
			SetFlag(AssetFlag::Invalid);
			return;
		}

		VkDeviceSize size = width * height * 4;

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;

		auto device = VulkanContext::GetCurrentDevice();

		stagingBufferMemory = Utility::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);
		VulkanAllocator allocator;

		void* data = allocator.MapMemory<void>(stagingBufferMemory);
		memcpy(data, imageData, static_cast<size_t>(size));
		allocator.UnmapMemory(stagingBufferMemory);

		stbi_image_free(imageData);

		ImageSpecification imageSpec{};
		imageSpec.format = ImageFormat::RGBA;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = (uint32_t)width;
		imageSpec.height = (uint32_t)height;

		m_image = std::reinterpret_pointer_cast<VulkanImage2D>(Image2D::Create(imageSpec));

		Utility::TransitionImageLayout(m_image->GetHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utility::CopyBufferToImage(stagingBuffer, m_image->GetHandle(), width, height);
		Utility::TransitionImageLayout(m_image->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		allocator.DestroyBuffer(stagingBuffer, stagingBufferMemory);

	}

	VulkanTexture2D::~VulkanTexture2D()
	{
	}

	void VulkanTexture2D::Bind(uint32_t slot) const
	{
	}

	void VulkanTexture2D::SetData(const void* data, uint32_t size)
	{
	}

	const uint32_t VulkanTexture2D::GetWidth() const
	{
		return m_image->GetSpecification().width;
	}

	const uint32_t VulkanTexture2D::GetHeight() const
	{
		return m_image->GetSpecification().height;
	}

	const uint32_t VulkanTexture2D::GetID() const
	{
		return uint32_t();
	}
}