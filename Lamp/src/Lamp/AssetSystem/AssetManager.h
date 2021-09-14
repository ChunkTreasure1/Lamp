


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

		AssetType GetAssetTypeFromPath(const std::filesystem::path& path);
		AssetType GetAssetTypeFromExtension(const std::string& ext);

	private:
		void LoaderThread();

	private:
		std::vector<std::thread> m_WorkerThreads;
		std::unordered_map<std::thread::id, std::string> m_ThreadNames;
		std::mutex m_OutputMutex;

		ThreadSafeQueue<AssetLoadJob> m_LoadingAssetsQueue;
		ThreadSafeQueue<AssetLoadJob> m_ProcessingAssetsQueue;

		uint32_t m_MaxThreads = 8;
		bool m_LoadingThreadActive = true;

		std::unordered_map<AssetType, Scope<AssetLoader>> m_AssetLoaders;
	};
}