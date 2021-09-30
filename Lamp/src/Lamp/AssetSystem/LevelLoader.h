#pragma once

#include <string>
#include "AssetLoader.h"

namespace Lamp
{
	class LevelLoader : public AssetLoader
	{
	public:
		virtual void Save(const Ref<Asset>& asset) const override;
		virtual bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
	};
}