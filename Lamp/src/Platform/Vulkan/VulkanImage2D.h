#pragma once

#include "Lamp/Rendering/Textures/Image2D.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	class VulkanImage2D : public Image2D
	{
	public:
		VulkanImage2D(const ImageSpecification& specification, const void* data);
		~VulkanImage2D();

		void Invalidate(const void* data) override;
		void Release() override;

		uint32_t GetWidth() const override { return m_specification.width; }
		uint32_t GetHeight() const override { return m_specification.height; }
		float GetAspectRatio() const override { return (float)m_specification.width / (float)m_specification.height; }

		inline const VkDescriptorImageInfo& GetDescriptorInfo() const { return m_descriptorInfo; }
		inline VkImage GetHandle() const { return m_image; }
		inline VkImageView GetImageView() const { return m_imageViews.at(0); }

		const ImageSpecification& GetSpecification() { return m_specification; }

	private:
		void UpdateDescriptor();

		ImageSpecification m_specification;

		VmaAllocation m_allocation = nullptr;
	
		VkDescriptorImageInfo m_descriptorInfo;
		VkImage m_image = nullptr;
		VkSampler m_sampler = nullptr;

		std::map<uint32_t, VkImageView> m_imageViews;
	};
}