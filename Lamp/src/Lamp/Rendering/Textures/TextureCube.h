#pragma once

#include "Texture.h"

namespace Lamp
{
	class Framebuffer;
	class TextureCube : public Texture
	{
	public:
		void Bind(uint32_t slot = 0) const override {}
		void SetData(const void* data, uint32_t size) override {}

		const uint32_t GetWidth() const override { return 0; };
		const uint32_t GetHeight() const override { return 0; };
		const uint32_t GetID() const override { return -1; };

		static Ref<TextureCube> Create(uint32_t width, uint32_t height, const std::vector<Ref<Framebuffer>>& textures);
		static Ref<TextureCube> Create(const std::filesystem::path& path);

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }
	};
}