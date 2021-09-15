#pragma once

#include <string>

#include <glad/glad.h>
#include "Lamp/Core/Core.h"

#include "Lamp/AssetSystem/Asset.h"

namespace Lamp
{
	class Texture2D : public Asset
	{
	public:
		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		//Getting
		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const uint32_t GetID() const = 0;

		static AssetType GetStaticType() { return AssetType::Texture; }
		virtual AssetType GetType() override { return GetStaticType(); }

	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(const std::filesystem::path& path);
	};
}