#pragma once

#include "Lamp/AssetSystem/Asset.h"

namespace Lamp
{
	class Texture : public Asset
	{
	public:
		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;

		//Getting
		virtual const uint32_t GetWidth() const = 0;
		virtual const uint32_t GetHeight() const = 0;
		virtual const uint32_t GetID() const = 0;

		static AssetType GetStaticType() { return AssetType::Texture; }
		AssetType GetType() override { return GetStaticType(); }
	};
}