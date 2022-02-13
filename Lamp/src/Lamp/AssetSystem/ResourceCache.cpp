#include "lppch.h"
#include "ResourceCache.h"

#include "AssetManager.h"

namespace Lamp
{
	std::unordered_map<std::filesystem::path, Ref<Asset>> ResourceCache::s_assetCache;
	std::mutex ResourceCache::s_cacheMutex;

	void ResourceCache::Shutdown()
	{
		s_assetCache.clear();
	}

	bool ResourceCache::AddAsset(const std::filesystem::path& path, Ref<Asset>& asset)
	{
		if (s_assetCache.find(path) == s_assetCache.end())
		{
			s_assetCache.insert(std::make_pair(path, asset));
			return true;
		}
		return false;
	}

	void ResourceCache::Update()
	{
		std::lock_guard<std::mutex> lock(s_cacheMutex);

		for (auto& asset : s_assetCache)
		{
			if (asset.second.use_count() <= 1)
			{
				s_assetCache.erase(asset.first);
				break;
			}
		}
	}
}