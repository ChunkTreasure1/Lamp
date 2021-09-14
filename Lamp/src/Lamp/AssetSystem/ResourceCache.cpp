#include "lppch.h"
#include "ResourceCache.h"

#include "AssetManager.h"

namespace Lamp
{
	std::unordered_map<std::filesystem::path, Ref<Asset>> ResourceCache::s_AssetCache;

	bool ResourceCache::AddAsset(const std::filesystem::path& path, Ref<Asset>& asset)
	{
		if (s_AssetCache.find(path) == s_AssetCache.end())
		{
			s_AssetCache.insert(std::make_pair(path, asset));
			return true;
		}
		return false;
	}

	Ref<Asset> ResourceCache::GetAsset(const std::filesystem::path& path)
	{
		if (s_AssetCache.find(path) != s_AssetCache.end())
		{
			return s_AssetCache.at(path);
		}

		Ref<Asset> asset;
		g_pEnv->pAssetManager->LoadAsset(path, asset);

		return asset;
	}
}