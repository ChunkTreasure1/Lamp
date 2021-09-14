#include "lppch.h"
#include "AssetManager.h"

#include "Lamp/Rendering/Texture2D/Texture2D.h"

#include "Lamp/AssetSystem/LevelLoader.h"

#include <glm/gtx/quaternion.hpp>

#include "ResourceCache.h"

namespace Lamp
{
	AssetManager::AssetManager()
	{
		Initialize();
	}

	AssetManager::~AssetManager()
	{
		Shutdown();
	}

	namespace Utils
	{
		std::string ToLower(const std::string& s)
		{
			std::string result;
			for (const auto& character : s)
			{
				result += std::tolower(character);
			}

			return result;
		}
	}

	void AssetManager::Initialize()
	{
		m_AssetLoaders[AssetType::Mesh] = CreateScope<MeshLoader>();
		m_AssetLoaders[AssetType::Texture] = CreateScope<TextureLoader>();
		m_AssetLoaders[AssetType::EnvironmentMap] = CreateScope<EnvironmentLoader>();
	}

	void AssetManager::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		m_LoadingThreadActive = false;
		for (uint32_t i = 0; i < m_MaxThreads; i++)
		{
			m_WorkerThreads[i].join();
		}
	}
	 
	void AssetManager::LoaderThread()
	{}

	void AssetManager::Update()
	{}

	void AssetManager::LoadAsset(const std::filesystem::path& path, Ref<Asset>& asset)
	{
		AssetLoadJob job;
		job.asset = asset;
		job.path = path;
		job.type = GetAssetTypeFromPath(path);

		if (m_AssetLoaders.find(job.type) == m_AssetLoaders.end())
		{
			LP_CORE_ERROR("No importer for asset exists!");
			return;
		}
		m_AssetLoaders[job.type]->Load(job.path, job.asset);
		if (asset->IsValid())
		{
			ResourceCache::AddAsset(path, asset);
		}
	}
	AssetType AssetManager::GetAssetTypeFromPath(const std::filesystem::path& path)
	{
		return GetAssetTypeFromExtension(path.extension().string());
	}
	AssetType AssetManager::GetAssetTypeFromExtension(const std::string& ext)
	{
		std::string ext = Utils::ToLower(ext);
		if (s_AssetExtensionMap.find(ext) == s_AssetExtensionMap.end())
		{
			return AssetType::None;
		}

		return s_AssetExtensionMap.at(ext);
	}
}