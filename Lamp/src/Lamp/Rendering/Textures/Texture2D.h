#pragma once

#include "Lamp/Rendering/Textures/Image2D.h"

#include "Lamp/AssetSystem/Asset.h"

namespace Lamp
{
	class Texture2D : public Asset
	{
	public:
		virtual ~Texture2D() = default;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const uint32_t GetID() const = 0;

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Texture2D> Create(ImageFormat format, uint32_t width, uint32_t height, bool saveBuffer = false);
		static Ref<Texture2D> Create(const std::filesystem::path& path, bool generateMips = true);
	};
}