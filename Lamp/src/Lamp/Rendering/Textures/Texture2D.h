#pragma once

#include "Texture.h"

namespace Lamp
{
	class Texture2D : public Texture
	{
	public:
		void Bind(uint32_t slot = 0) const override {}
		void SetData(const void* data, uint32_t size) override {}

		const uint32_t GetWidth() const override { return 0; };
		const uint32_t GetHeight() const override { return 0; };
		const uint32_t GetID() const override { return -1; };

		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::filesystem::path& path);

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }
	};
}