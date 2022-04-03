#include "lppch.h"
#include "Image2D.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanUtility.h"

namespace Lamp
{
	Ref<Image2D> Image2D::Create(const ImageSpecification& specification, const void* data)
	{
		return CreateRef<Image2D>(specification, data);
	}

	Image2D::Image2D(const ImageSpecification& specification, const void* data)
		: m_specification(specification)
	{
		Invalidate(data);
	}

	Image2D::~Image2D()
	{
		Release();
	}

	void Image2D::Invalidate(const void* data)
	{
		Release();

		VulkanAllocator allocator;
		auto device = VulkanContext::GetCurrentDevice();

		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		if (m_specification.copyable)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		if (m_specification.usage == ImageUsage::Attachment)
		{
			if (Utils::IsDepthFormat(m_specification.format))
			{
				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
			else
			{
				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
		}
		else if (m_specification.usage == ImageUsage::Texture)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		else if (m_specification.usage == ImageUsage::Storage)
		{
			usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		VkImageAspectFlags aspectMask = Utils::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (m_specification.format == ImageFormat::DEPTH24STENCIL8)
		{
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		m_format = Utility::LampFormatToVulkanFormat(m_specification.format);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.usage = usage;
		imageInfo.extent.width = m_specification.width;
		imageInfo.extent.height = m_specification.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = m_specification.mips;
		imageInfo.arrayLayers = m_specification.layers;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.format = m_format;

		m_allocation = allocator.AllocateImage(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, m_image);

		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = m_specification.layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = m_format;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.subresourceRange = {};
		imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = m_specification.mips;
		imageViewCreateInfo.subresourceRange.layerCount = m_specification.layers;
		imageViewCreateInfo.image = m_image;

		LP_VK_CHECK(vkCreateImageView(device->GetHandle(), &imageViewCreateInfo, nullptr, &m_imageViews[0]));

		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.anisotropyEnable = m_specification.useAniostopy && Renderer::Get().GetCapabilities().supportAniostopy ? VK_TRUE : VK_FALSE;
		samplerCreateInfo.maxAnisotropy = (uint32_t)m_specification.useAniostopy > Renderer::Get().GetCapabilities().maxAniostropy ? (float)Renderer::Get().GetCapabilities().maxAniostropy : (float)m_specification.useAniostopy;
		samplerCreateInfo.magFilter = Utility::LampFilterToVulkanFilter(m_specification.filter);
		samplerCreateInfo.minFilter = Utility::LampFilterToVulkanFilter(m_specification.filter);
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeV = Utility::LampWrapToVulkanWrap(m_specification.wrap);
		samplerCreateInfo.addressModeU = Utility::LampWrapToVulkanWrap(m_specification.wrap);
		samplerCreateInfo.addressModeW = Utility::LampWrapToVulkanWrap(m_specification.wrap);
		samplerCreateInfo.mipLodBias = 0.f;
		samplerCreateInfo.minLod = 0.f;
		samplerCreateInfo.maxLod = 3.f; //TODO: maybe set somewhere else?
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerCreateInfo.compareEnable = m_specification.comparable ? VK_TRUE : VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_LESS;

		LP_VK_CHECK(vkCreateSampler(device->GetHandle(), &samplerCreateInfo, nullptr, &m_sampler));

		UpdateDescriptor();
	}

	void Image2D::Release()
	{
		if (!m_image && !m_allocation)
		{
			return;
		}

		auto device = VulkanContext::GetCurrentDevice();
		vkDeviceWaitIdle(device->GetHandle());

		vkDestroySampler(device->GetHandle(), m_sampler, nullptr);

		for (const auto& imageView : m_imageViews)
		{
			vkDestroyImageView(device->GetHandle(), imageView.second, nullptr);
		}

		m_imageViews.clear();

		VulkanAllocator allocator;
		allocator.DestroyImage(m_image, m_allocation);

		m_image = nullptr;
		m_allocation = nullptr;
	}

	void Image2D::TransitionToLayout(Ref<CommandBuffer> commandBuffer, VkImageLayout layout)
	{
		VkImageAspectFlags flag = Utils::IsDepthFormat(m_specification.format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageSubresourceRange subRange{ flag, 0, 1, 0, 1 };
		Utility::TransitionImageLayout(commandBuffer->GetCurrentCommandBuffer(), m_image, m_imageLayout, layout, subRange);
		m_imageLayout = layout;
	}

	void Image2D::UpdateDescriptor()
	{
		if (m_specification.format == ImageFormat::DEPTH24STENCIL8 || m_specification.format == ImageFormat::DEPTH32F)
		{
			m_descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}
		else if (m_specification.usage == ImageUsage::Storage)
		{
			m_descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		}
		else
		{
			m_descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		m_descriptorInfo.imageView = m_imageViews[0];
		m_descriptorInfo.sampler = m_sampler;

		m_imageLayout = m_descriptorInfo.imageLayout;
	}
}