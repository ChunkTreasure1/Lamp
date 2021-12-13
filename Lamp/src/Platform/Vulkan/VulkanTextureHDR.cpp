#include "lppch.h"
#include "VulkanTextureHDR.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanUtility.h"

#include <stb/stb_image.h>

namespace Lamp
{
	VulkanTextureHDR::VulkanTextureHDR(uint32_t width, uint32_t height)
	{
		ImageSpecification imageSpec{};
		imageSpec.format = ImageFormat::RGBA32F;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = (uint32_t)width;
		imageSpec.height = (uint32_t)height;

		m_image = std::reinterpret_pointer_cast<VulkanImage2D>(Image2D::Create(imageSpec));
	}

	VulkanTextureHDR::VulkanTextureHDR(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			LP_CORE_ERROR("VulkanTextureHDR: File {0} not found!", path.string());
			SetFlag(AssetFlag::Missing);
			return;
		}

		int width;
		int height;
		int channels;
		float* imageData = stbi_loadf(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
	
		if (!imageData)
		{
			LP_CORE_ERROR("VulkanTextureHDR: File {0} could not be loaded!", path.string());
			SetFlag(AssetFlag::Missing);
			return;
		}

		VkDeviceSize size = width * height * 4 * sizeof(4);
		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;
		VulkanAllocator allocator;

		auto device = VulkanContext::GetCurrentDevice();
		stagingBufferMemory = Utility::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);

		void* data = allocator.MapMemory<void>(stagingBufferMemory);
		memcpy(data, imageData, static_cast<uint32_t>(size));
		allocator.UnmapMemory(stagingBufferMemory);

		stbi_image_free(imageData);

		ImageSpecification imageSpec{};
		imageSpec.format = ImageFormat::RGBA16F;
		imageSpec.usage = ImageUsage::Texture;
		imageSpec.width = (uint32_t)width;
		imageSpec.height = (uint32_t)height;
		imageSpec.mips = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

		m_image = std::reinterpret_pointer_cast<VulkanImage2D>(Image2D::Create(imageSpec));

		Utility::TransitionImageLayout(m_image->GetHandle(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utility::CopyBufferToImage(stagingBuffer, m_image->GetHandle(), width, height);
		Utility::GenerateMipMaps(m_image->GetHandle(), width, height, imageSpec.mips);

		allocator.DestroyBuffer(stagingBuffer, stagingBufferMemory);
	}

	VulkanTextureHDR::~VulkanTextureHDR()
	{
	}
	
	void VulkanTextureHDR::Bind(uint32_t slot) const
	{
	}
	
	void VulkanTextureHDR::SetData(const void* data, uint32_t size)
	{
	}
}