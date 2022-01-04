#include "lppch.h"
#include "VulkanTexture2D.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtility.h"

#include <stb/stb_image.h>

namespace Lamp
{
	VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height)
	{
		ImageSpecification imageSpec{};
		imageSpec.format = ImageFormat::RGBA32F;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = (uint32_t)width;
		imageSpec.height = (uint32_t)height;

		m_image = std::reinterpret_pointer_cast<VulkanImage2D>(Image2D::Create(imageSpec));
	}

	VulkanTexture2D::VulkanTexture2D(const std::filesystem::path& path, bool generateMips)
	{
		if (!std::filesystem::exists(path))
		{
			LP_CORE_ERROR("File not found {0}", path.string());
			SetFlag(AssetFlag::Missing);
			return;
		}

		VkDeviceSize size;
		ImageFormat format;
		void* imageData = nullptr;

		int width;
		int height;
		int channels;

		if (stbi_is_hdr(path.string().c_str()))
		{
			stbi_set_flip_vertically_on_load(0);
			imageData = stbi_loadf(path.string().c_str(), &width, &height, &channels, 4);
			size = width * height * 4 * sizeof(float);
		}
		else
		{
			stbi_set_flip_vertically_on_load(1);
			imageData = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
			size = width * height * 4;
		}


		if (!imageData)
		{
			LP_CORE_ERROR("Unable to load texture {0}", path.string());
			SetFlag(AssetFlag::Invalid);
			return;
		}

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
		imageSpec.format = stbi_is_hdr(path.string().c_str()) ? ImageFormat::RGBA32F : ImageFormat::RGBA;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = (uint32_t)width;
		imageSpec.height = (uint32_t)height;
		imageSpec.mips = generateMips ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;

		m_image = std::reinterpret_pointer_cast<VulkanImage2D>(Image2D::Create(imageSpec));

		Utility::TransitionImageLayout(m_image->GetHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utility::CopyBufferToImage(stagingBuffer, m_image->GetHandle(), width, height);
		Utility::GenerateMipMaps(m_image->GetHandle(), width, height, imageSpec.mips);

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
		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;

		auto device = VulkanContext::GetCurrentDevice();

		VkDeviceSize deviceSize = size;

		stagingBufferMemory = Utility::CreateBuffer(deviceSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);
		VulkanAllocator allocator;

		void* newData = allocator.MapMemory<void>(stagingBufferMemory);
		memcpy(newData, data, size);
		allocator.UnmapMemory(stagingBufferMemory);

		Utility::TransitionImageLayout(m_image->GetHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utility::CopyBufferToImage(stagingBuffer, m_image->GetHandle(), m_image->GetWidth(), m_image->GetHeight());
		Utility::TransitionImageLayout(m_image->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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