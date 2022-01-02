#pragma once

#include "Lamp/Rendering/Textures/TextureCube.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	class VulkanTextureCube : public TextureCube
	{
	public:
		VulkanTextureCube(uint32_t width, uint32_t height);
		VulkanTextureCube(const std::filesystem::path& path);

		~VulkanTextureCube();

		void Bind(uint32_t slot /* = 0 */) const override;
		void SetData(const void* data, uint32_t size) override;
		void SetData(Ref<Image2D> image, uint32_t face, uint32_t mip) override;

		const uint32_t GetWidth() const override { return m_width; }
		const uint32_t GetHeight() const override { return m_height; }
		const uint32_t GetID() const override { return 0; }

		inline VkDescriptorImageInfo& GetDescriptorInfo() { return m_descriptorInfo; }

		void StartDataOverride() override;
		void FinishDataOverride() override;

	private:
		void UpdateDescriptor();

		void Invalidate();
		void Release();

		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_mipLevels;
	
		VmaAllocation m_allocation = nullptr;
		VkDescriptorImageInfo m_descriptorInfo;
		VkImage m_image = nullptr;
		VkSampler m_sampler;

		VkImageView m_imageView;
	};
}