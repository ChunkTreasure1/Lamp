#pragma once

#include "Lamp/Rendering/Textures/TextureCube.h"
#include "Lamp/Core/Buffer.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>

namespace Lamp
{
	class VulkanTextureCube : public TextureCube
	{
	public:
		VulkanTextureCube(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
		VulkanTextureCube(const std::filesystem::path& path);

		~VulkanTextureCube() override;

		void Bind(uint32_t slot /* = 0 */) const override;
		void SetData(const void* data, uint32_t size) override;
		void SetData(Ref<Image2D> image, uint32_t face, uint32_t mip) override;

		const uint32_t GetWidth() const override { return m_width; }
		const uint32_t GetHeight() const override { return m_height; }
		const uint32_t GetID() const override { return 0; }

		const uint32_t GetMipLevelCount() const override;
		inline VkImage GetImage() const { return m_image; }

		VkImageView CreateImageViewSingleMip(uint32_t mip);

		inline VkDescriptorImageInfo& GetDescriptorInfo() { return m_descriptorInfo; }

		void StartDataOverride() override;
		void FinishDataOverride() override;

		void GenerateMips(bool readOnly);

	private:

		void Invalidate();
		void Release();

		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_mipLevels;

		ImageFormat m_format;
		Buffer m_localBuffer;
	
		VmaAllocation m_allocation = nullptr;
		VkDescriptorImageInfo m_descriptorInfo;
		VkImage m_image = nullptr;

		bool m_mipsGenerated = false;
	};
}