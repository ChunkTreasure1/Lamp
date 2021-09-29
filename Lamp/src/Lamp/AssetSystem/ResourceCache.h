#pragma once

#include <unordered_map>
#include "Lamp/Meshes/Mesh.h"

#include "Lamp/AssetSystem/AssetManager.h"

namespace Lamp
{
	class ResourceCache
	{
	public:
		static bool AddAsset(std::filesystem::path path, Ref<Asset>& asset);

		static void Update();

		template<typename T>
		static Ref<T> GetAsset(std::filesystem::path path)
		{

			if (s_AssetCache.find(path.string()) != s_AssetCache.end())
			{
				return std::dynamic_pointer_cast<T>(s_AssetCache.at(path.string()));
			}

			Ref<Asset> asset;
			g_pEnv->pAssetManager->LoadAsset(path, asset);

			AddAsset(path, asset);

			return std::dynamic_pointer_cast<T>(asset);
		}

		template<typename T>
		static Ref<T> ReloadAsset(Ref<Asset>& asset)
		{
			Ref<Asset> newAsset;
			g_pEnv->pAssetManager->LoadAsset(asset->Path, newAsset);

			s_AssetCache[asset->Path.string()] = newAsset;

			return std::dynamic_pointer_cast<T>(newAsset);
		}

	private:
		ResourceCache() = delete;

	private:
		static std::unordered_map<std::string, Ref<Asset>> s_AssetCache;
		static std::mutex s_CacheMutex;
	};
}