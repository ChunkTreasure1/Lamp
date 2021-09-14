#pragma once

#include <unordered_map>
#include "Lamp/Meshes/Mesh.h"

namespace Lamp
{
	class ResourceCache
	{
	public:
		static bool AddAsset(const std::filesystem::path& path, Ref<Asset>& asset);

		static Ref<Asset> GetAsset(const std::filesystem::path& path);

	private:
		ResourceCache() = delete;

	private:
		static std::unordered_map<std::filesystem::path, Ref<Asset>> s_AssetCache;
	};
}