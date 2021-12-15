#pragma once

#include "Lamp/AssetSystem/Asset.h"

namespace Lamp
{
	class Image2D;
	class TextureCube : public Asset
	{
	public:
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void SetData(Ref<Image2D> image, uint32_t face, uint32_t mip) = 0;

		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const uint32_t GetID() const = 0;

		virtual void StartDataOverride() = 0;
		virtual void FinishDataOverride() = 0;

		static Ref<TextureCube> Create(uint32_t width, uint32_t height);
		static Ref<TextureCube> Create(const std::filesystem::path& path);

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }
	};
}