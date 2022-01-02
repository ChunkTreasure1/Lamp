#include "lppch.h"
#include "VulkanTextureCube.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanAllocator.h"

#include "Platform/Vulkan/VulkanUtility.h"
#include "Platform/Vulkan/VulkanImage2D.h"

namespace Lamp
{
	VulkanTextureCube::VulkanTextureCube(uint32_t width, uint32_t height)
		: m_width(width), m_height(height)
	{
		Invalidate();
	}

	VulkanTextureCube::VulkanTextureCube(const std::filesystem::path& path)
	{
		//TODO: implement
	}

	VulkanTextureCube::~VulkanTextureCube()
	{
		Release();
	}

	void VulkanTextureCube::Bind(uint32_t slot) const
	{
	}

	void VulkanTextureCube::SetData(const void* data, uint32_t size)
	{

	}

	void VulkanTextureCube::SetData(Ref<Image2D> image, uint32_t face, uint32_t mip)
	{
		auto device = VulkanContext::GetCurrentDevice();
		auto vulkanImage = std::reinterpret_pointer_cast<VulkanImage2D>(image);
		auto commandBuffer = device->GetCommandBuffer(true);
		
		Utility::TransitionImageLayout(vulkanImage->GetHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		VkImageCopy copyRegion{};
		
		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffset = { 0, 0, 0 };

		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.baseArrayLayer = face;
		copyRegion.dstSubresource.mipLevel = mip;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffset = { 0, 0, 0 };

		copyRegion.extent.width = static_cast<uint32_t>(m_width * glm::pow(0.5f, mip));
		copyRegion.extent.height = static_cast<uint32_t>(m_height * glm::pow(0.5f, mip));
		copyRegion.extent.depth = 1;

		vkCmdCopyImage(commandBuffer, vulkanImage->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		device->FlushCommandBuffer(commandBuffer);

		Utility::TransitionImageLayout(vulkanImage->GetHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

	void VulkanTextureCube::StartDataOverride()
	{
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = m_mipLevels;
		subresourceRange.layerCount = 6;

		Utility::TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
	}

	void VulkanTextureCube::FinishDataOverride()
	{
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = m_mipLevels;
		subresourceRange.layerCount = 6;

		Utility::TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);
	}

	void VulkanTextureCube::UpdateDescriptor()
	{
		m_descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_descriptorInfo.imageView = m_imageView;
		m_descriptorInfo.sampler = m_sampler;
	}

	void VulkanTextureCube::Invalidate()
	{
		Release();

		auto device = VulkanContext::GetCurrentDevice();

		//TODO: fix
		m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_height, m_width)))) + 1;
		const VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		imageInfo.extent.width = m_width;
		imageInfo.extent.height = m_height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = m_mipLevels;
		imageInfo.arrayLayers = 6;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.format = format;
	
		VulkanAllocator allocator;
		m_allocation = allocator.AllocateImage(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_image);

		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		imageViewInfo.format = format;
		imageViewInfo.subresourceRange = {};
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.levelCount = m_mipLevels;
		imageViewInfo.subresourceRange.layerCount = 6;
		imageViewInfo.image = m_image;

		VkResult result = vkCreateImageView(device->GetHandle(), &imageViewInfo, nullptr, &m_imageView);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create image view");

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.minLod = 0.f;
		samplerInfo.maxLod = static_cast<float>(m_mipLevels);
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		result = vkCreateSampler(device->GetHandle(), &samplerInfo, nullptr, &m_sampler);

		UpdateDescriptor();
	}

	void VulkanTextureCube::Release()
	{
		if (!m_image && !m_allocation)
		{
			return;
		}

		auto device = VulkanContext::GetCurrentDevice();
		vkDeviceWaitIdle(device->GetHandle());

		vkDestroySampler(device->GetHandle(), m_sampler, nullptr);
		vkDestroyImageView(device->GetHandle(), m_imageView, nullptr);

		VulkanAllocator allocator;
		allocator.DestroyImage(m_image, m_allocation);

		m_image = nullptr;
		m_allocation = nullptr;
	}
}