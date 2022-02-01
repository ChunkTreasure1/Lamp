#include "lppch.h"
#include "VulkanTextureCube.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanAllocator.h"

#include "Platform/Vulkan/VulkanUtility.h"
#include "Platform/Vulkan/VulkanImage2D.h"

namespace Lamp
{
	VulkanTextureCube::VulkanTextureCube(ImageFormat format, uint32_t width, uint32_t height, const void* data)
		: m_width(width), m_height(height), m_format(format)
	{
		if (data)
		{
			uint32_t size = width * height * 4 * 6;
			m_localBuffer = Buffer::Copy(data, size);
		}

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

	const uint32_t VulkanTextureCube::GetMipLevelCount() const
	{
		return Utility::CalculateMipCount(m_width, m_height);
	}

	VkImageView VulkanTextureCube::CreateImageViewSingleMip(uint32_t mip)
	{
		auto device = VulkanContext::GetCurrentDevice();
	
		VkFormat format = Utility::LampFormatToVulkanFormat(m_format);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = format;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = mip;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.image = m_image;

		VkImageView result;
		VkResult vkResult = vkCreateImageView(device->GetHandle(), &viewInfo, nullptr, &result);
		LP_CORE_ASSERT(vkResult == VK_SUCCESS, "Unable to create image view!");

		return result;
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

	void VulkanTextureCube::GenerateMips(bool readOnly)
	{
		auto device = VulkanContext::GetCurrentDevice();
	
		VkCommandBuffer blitCmd = VulkanContext::GetCurrentDevice()->GetCommandBuffer(true);

		uint32_t mipLevels = GetMipLevelCount();
		for (uint32_t face = 0; face < 6; face++)
		{
			VkImageSubresourceRange mipSubRange{};
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = 0;
			mipSubRange.baseArrayLayer = face;
			mipSubRange.levelCount = 1;
			mipSubRange.layerCount = 1;
	
			Utility::InsertImageMemoryBarrier(blitCmd, m_image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mipSubRange);
		}

		for (uint32_t i = 1; i < mipLevels; i++)
		{
			for (uint32_t face = 0; face < 6; face++)
			{
				VkImageBlit imageBlit{};
				imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageBlit.srcSubresource.layerCount = 1;
				imageBlit.srcSubresource.mipLevel = i - 1;
				imageBlit.srcSubresource.baseArrayLayer = face;

				imageBlit.srcOffsets[1].x = int32_t(m_width >> (i - 1));
				imageBlit.srcOffsets[1].y = int32_t(m_height >> (i - 1));
				imageBlit.srcOffsets[1].z = 1;

				imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageBlit.dstSubresource.layerCount = 1;
				imageBlit.dstSubresource.mipLevel = i;
				imageBlit.dstSubresource.baseArrayLayer = face;

				imageBlit.dstOffsets[1].x = int32_t(m_width >> i);
				imageBlit.dstOffsets[1].y = int32_t(m_height >> i);
				imageBlit.dstOffsets[1].z = 1;

				VkImageSubresourceRange mipSubRange{};
				mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				mipSubRange.baseMipLevel = i;
				mipSubRange.baseArrayLayer = face;
				mipSubRange.levelCount = 1;
				mipSubRange.layerCount = 1;

				Utility::InsertImageMemoryBarrier(blitCmd, m_image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mipSubRange);

				vkCmdBlitImage(blitCmd, m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

				Utility::InsertImageMemoryBarrier(blitCmd, m_image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, mipSubRange);
			}
		}

		//Transition all mips to shader read only
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.layerCount = 6;
		subresourceRange.levelCount = mipLevels;

		Utility::InsertImageMemoryBarrier(blitCmd, m_image, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readOnly ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, subresourceRange);
	
		VulkanContext::GetCurrentDevice()->FlushCommandBuffer(blitCmd);
		m_mipsGenerated = true;
		m_descriptorInfo.imageLayout = readOnly ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
	}

	void VulkanTextureCube::Invalidate()
	{
		Release();

		auto device = VulkanContext::GetCurrentDevice();

		const uint32_t mipCount = GetMipLevelCount();
		const VkFormat format = Utility::LampFormatToVulkanFormat(m_format);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format;
		imageInfo.mipLevels = mipCount;
		imageInfo.arrayLayers = 6;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.extent = { m_width, m_height, 1 };
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VulkanAllocator allocator;
		m_allocation = allocator.AllocateImage(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_image);

		m_descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		//TODO: copy data
		if (m_localBuffer)
		{
			VkBuffer stagingBuffer;
			VmaAllocation stagingBufferMemory;

			stagingBufferMemory = Utility::CreateBuffer(m_localBuffer.Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer);
			
			uint8_t* data = allocator.MapMemory<uint8_t>(stagingBufferMemory);
			memcpy(data, m_localBuffer.pData, m_localBuffer.Size);
			allocator.UnmapMemory(m_allocation);

			Utility::TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			Utility::CopyBufferToImage(stagingBuffer, m_image, m_width, m_height);

			Utility::TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_descriptorInfo.imageLayout);
			allocator.DestroyBuffer(stagingBuffer, stagingBufferMemory);
		}

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = mipCount;
		subresourceRange.layerCount = 6;

		Utility::TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_UNDEFINED, m_descriptorInfo.imageLayout, subresourceRange);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.maxAnisotropy = 1.f;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0.f;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.f;
		samplerInfo.maxLod = (float)mipCount;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VkResult result = vkCreateSampler(device->GetHandle(), &samplerInfo, nullptr, &m_descriptorInfo.sampler);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create sampler");

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = format;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;
		viewInfo.subresourceRange.levelCount = mipCount;
		viewInfo.image = m_image;

		result = vkCreateImageView(device->GetHandle(), &viewInfo, nullptr, &m_descriptorInfo.imageView);
		LP_CORE_ASSERT(result == VK_SUCCESS, "Unable to create image view");
	}

	void VulkanTextureCube::Release()
	{
		if (!m_image && !m_allocation)
		{
			return;
		}

		auto device = VulkanContext::GetCurrentDevice();
		vkDeviceWaitIdle(device->GetHandle());

		vkDestroyImageView(device->GetHandle(), m_descriptorInfo.imageView, nullptr);
		vkDestroySampler(device->GetHandle(), m_descriptorInfo.sampler, nullptr);

		VulkanAllocator allocator;
		allocator.DestroyImage(m_image, m_allocation);

		m_image = nullptr;
		m_allocation = nullptr;
	}
}