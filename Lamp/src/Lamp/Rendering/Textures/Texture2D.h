#pragma once

#include "Lamp/Core/Buffer.h"

#include "Lamp/Rendering/Textures/Image2D.h"

#include "Lamp/AssetSystem/Asset.h"

namespace Lamp
{
	class Texture2D : public Asset
	{
	public:
		Texture2D(ImageFormat format, uint32_t width, uint32_t height, bool saveBuffer);
		Texture2D(const std::filesystem::path& path, bool generateMips);
		Texture2D() = default;

		~Texture2D();

		void Bind(uint32_t slot /* = 0 */) const;
		void SetData(const void* data, uint32_t size);

		const uint32_t GetWidth() const;
		const uint32_t GetHeight() const;
		const uint32_t GetID() const;

		void Load(const std::filesystem::path & path, bool generateMips /* = true */);

		const VkDescriptorImageInfo& GetDescriptorInfo() const;
		inline const Ref<Image2D> GetImage() const { return m_image; }
		inline Buffer GetData() const { return m_buffer; }

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() { return GetStaticType(); }

		static Ref<Texture2D> Create(ImageFormat format, uint32_t width, uint32_t height, bool saveBuffer = false);
		static Ref<Texture2D> Create(const std::filesystem::path& path, bool generateMips = true);

	private:
		void LoadKTX(const std::filesystem::path & path, bool generateMips);
		void LoadOther(const std::filesystem::path & path, bool generateMips);
		void Setup(void* data, uint32_t size, uint32_t width, uint32_t height, bool generateMips, bool isHDR);

		Ref<Image2D> m_image;
		Buffer m_buffer;
		bool m_saveBuffer = false;
	};
}