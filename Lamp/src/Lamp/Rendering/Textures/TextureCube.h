#pragma once

#include "Lamp/AssetSystem/Asset.h"

#include "Lamp/Rendering/Textures/Image2D.h"
#include "Lamp/Core/Buffer.h"

namespace Lamp
{
	class TextureCube : public Asset
	{
	public:
		TextureCube(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
		TextureCube(const std::filesystem::path& path);

		~TextureCube();

		void SetData(const void* data, uint32_t size);
		void SetData(Ref<Image2D> image, uint32_t face, uint32_t mip);

		const uint32_t GetWidth() const { return m_width; }
		const uint32_t GetHeight() const { return m_height; }

		const uint32_t GetMipLevelCount() const;
		inline VkImage GetImage() const { return m_image; }

		VkImageView CreateImageViewSingleMip(uint32_t mip);

		inline VkDescriptorImageInfo& GetDescriptorInfo() { return m_descriptorInfo; }

		void GenerateMips(bool readOnly);

		static Ref<TextureCube> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
		static Ref<TextureCube> Create(const std::filesystem::path& path);

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }

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