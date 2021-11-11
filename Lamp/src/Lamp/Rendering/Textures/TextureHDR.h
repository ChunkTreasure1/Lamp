#pragma once

#include "Lamp/AssetSystem/Asset.h"

namespace Lamp
{
	class TextureHDR : public Asset
	{
	public:
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const uint32_t GetID() const = 0;

		static Ref<TextureHDR> Create(uint32_t width, uint32_t height);
		static Ref<TextureHDR> Create(const std::filesystem::path& path);

		static AssetType GetStaticType() { return AssetType::EnvironmentMap; }
		AssetType GetType() override { return GetStaticType(); }
	};
}