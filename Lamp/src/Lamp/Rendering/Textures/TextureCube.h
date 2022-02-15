#pragma once

#include "Lamp/AssetSystem/Asset.h"

#include "Lamp/Rendering/Textures/Image2D.h"

namespace Lamp
{
	class TextureCube : public Asset
	{
	public:
		virtual ~TextureCube() = default;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void SetData(Ref<Image2D> image, uint32_t face, uint32_t mip) = 0;

		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const uint32_t GetID() const = 0;

		virtual void StartDataOverride() = 0;
		virtual void FinishDataOverride() = 0;

		virtual const uint32_t GetMipLevelCount() const = 0;

		static Ref<TextureCube> Create(ImageFormat format, uint32_t width, uint32_t height, const void* data = nullptr);
		static Ref<TextureCube> Create(const std::filesystem::path& path);

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }
	};
}