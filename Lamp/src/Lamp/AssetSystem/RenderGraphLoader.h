#pragma once

#include "AssetLoader.h"

#include <yaml-cpp/yaml.h>

namespace Lamp
{
	struct RenderNode;

	class RenderGraphLoader : public AssetLoader
	{
	public:
		virtual void Save(const Ref<Asset>& asset) const override;
		virtual bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;

	};
}