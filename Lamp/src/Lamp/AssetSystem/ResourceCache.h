#pragma once

#include "Lamp/AssetSystem/AssetManager.h"
#include "Lamp/Rendering/Textures/Texture2D.h"

#include <unordered_map>
#include <mutex>

namespace Sandbox
{
	class AssetBrowserPanel;
}

namespace std
{
	template<>
	struct hash<std::filesystem::path>
	{
		std::size_t operator()(const std::filesystem::path& path) const
		{
			return hash_value(path);
		}
	};
}

namespace Lamp
{
	class ResourceCache
	{
	public:
		static bool AddAsset(const std::filesystem::path& path, Ref<Asset>& asset);

		static void Update();

		template<typename T>
		static Ref<T> GetAsset(const std::filesystem::path& path)
		{
			if (s_AssetCache.find(path) != s_AssetCache.end())
			{
				return std::dynamic_pointer_cast<T>(s_AssetCache.at(path));
			}

			Ref<Asset> asset = g_pEnv->pAssetManager->GetAsset<T>(path);

			AddAsset(path, asset);

			return std::dynamic_pointer_cast<T>(asset);
		}

		template<typename T>
		static Ref<T> ReloadAsset(Ref<Asset>& asset)
		{
			Ref<Asset> newAsset;
			g_pEnv->pAssetManager->LoadAsset(asset->Path, newAsset);

			s_AssetCache[asset->Path] = newAsset;

			return std::dynamic_pointer_cast<T>(newAsset);
		}

	private:
		ResourceCache() = delete;

	private:
		friend class Sandbox::AssetBrowserPanel;

		static std::unordered_map<std::filesystem::path, Ref<Asset>> s_AssetCache;
		static std::mutex s_CacheMutex;
	};
}