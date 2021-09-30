


#pragma once

#include <thread>
#include <vector>
#include <unordered_map>

#include "Lamp/Utility/ThreadSafeQueue.h"
#include "Lamp/Meshes/Mesh.h"
#include "Lamp/Level/Level.h"
#include "AssetLoader.h"

namespace Lamp
{
	struct AssetLoadJob
	{
		std::filesystem::path path;
		Ref<Asset> asset;
		AssetType type;
	};

	class AssetManager
	{
	public:
		AssetManager();
		~AssetManager();

		void Initialize();
		void Shutdown();

		void Update();

		void LoadAsset(const std::filesystem::path& path, Ref<Asset>& asset);
		void SaveAsset(const Ref<Asset>& asset);

		AssetType GetAssetTypeFromPath(const std::filesystem::path& path);
		AssetType GetAssetTypeFromExtension(const std::string& ext);
		AssetHandle GetAssetHandleFromPath(const std::filesystem::path& path);
		std::filesystem::path GetPathFromAssetHandle(AssetHandle handle);

		template<typename T>
		Ref<T> GetAsset(AssetHandle assetHandle)
		{
			Ref<Asset> asset = nullptr;

			std::filesystem::path assetPath = GetPathFromAssetHandle(assetHandle);

			if (std::filesystem::exists(assetPath))
			{
				LoadAsset(assetPath, asset);
				asset->Handle = assetHandle;
				asset->Path = assetPath;

				return std::dynamic_pointer_cast<T>(asset);
			}

			return nullptr;
		}

		template<typename T>
		Ref<T> GetAsset(const std::filesystem::path& path)
		{
			Ref<Asset> asset = GetAsset<T>(GetAssetHandleFromPath(path));
			if (asset == nullptr)
			{
				LoadAsset(path, asset);
				m_AssetRegistry.emplace(std::make_pair(path, asset->Handle));
			}
			return std::dynamic_pointer_cast<T>(asset);
		}

	private:
		void SaveAssetRegistry();
		void LoadAssetRegistry();

	private:

		std::unordered_map<AssetType, Scope<AssetLoader>> m_AssetLoaders;
		std::map<std::filesystem::path, AssetHandle> m_AssetRegistry;
	};
}