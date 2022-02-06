#pragma once

#include "AssetLoader.h"

#include "Lamp/Utility/ThreadSafeQueue.h"

#include <thread>
#include <vector>
#include <unordered_map>
#include <map>

namespace Lamp
{
	struct AssetLoadJob
	{
		AssetLoadJob(const std::filesystem::path& aPath, AssetType aType, AssetHandle aHandle, Ref<Asset>& aAsset)
			: path(aPath), type(aType), asset(&aAsset), handle(aHandle)
		{}

		AssetLoadJob(const AssetLoadJob& job)
			: path(job.path), asset(job.asset), type(job.type), 
			finished(job.finished), threadId(job.threadId), handle(job.handle)
		{ }

		AssetLoadJob& operator=(const AssetLoadJob& job)
		{
			path = job.path;
			asset = job.asset;
			type = job.type;
			finished = job.finished;
			threadId = job.threadId;
			handle = job.handle;

			return *this;
		}

		std::filesystem::path path;
		Ref<Asset>* asset;
		AssetType type;
		AssetHandle handle;

		bool finished = false;
		std::thread::id threadId;
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
				asset = T::Create();
				LoadAsset(assetPath, asset);

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
				asset = T::Create();
				LoadAsset(path, asset);
			}
			return std::dynamic_pointer_cast<T>(asset);
		}

	private:
		void SaveAssetRegistry();
		void LoadAssetRegistry();

		void Thread_LoadAsset(Ref<AssetLoadJob>& loadJob);

		std::unordered_map<AssetType, Scope<AssetLoader>> m_assetLoaders;
		std::map<std::filesystem::path, AssetHandle> m_assetRegistry;

		const uint32_t m_maxThreads = 8;
		std::vector<std::thread> m_threadPool;

		ThreadSafeQueue<AssetLoadJob> m_loadQueue;
		std::vector<Ref<AssetLoadJob>> m_loadingAssets;
	};
}