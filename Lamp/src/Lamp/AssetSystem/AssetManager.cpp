#include "lppch.h"
#include "AssetManager.h"

#include "Lamp/Rendering/Texture2D/Texture2D.h"

#include "Lamp/AssetSystem/LevelLoader.h"
#include "Lamp/AssetSystem/RenderGraphLoader.h"

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
		m_AssetLoaders[AssetType::RenderGraph] = CreateScope<RenderGraphLoader>();
		m_AssetLoaders[AssetType::Material] = CreateScope<MaterialLoader>();
	}

	void AssetManager::Shutdown()
	{
		LP_PROFILE_FUNCTION();
	}
	

	void AssetManager::Update()
	{
		ResourceCache::Update();
	}

	void AssetManager::LoadAsset(const std::filesystem::path& path, Ref<Asset>& asset)
	{
		AssetLoadJob job;
		job.path = path;
		job.type = GetAssetTypeFromPath(path);

		if (m_AssetLoaders.find(job.type) == m_AssetLoaders.end())
		{
			LP_CORE_ERROR("No importer for asset exists!");
			return;
		}
		m_AssetLoaders[job.type]->Load(job.path, asset);
	}

	void AssetManager::SaveAsset(const Ref<Asset>& asset)
	{
		if (m_AssetLoaders.find(asset->GetType()) == m_AssetLoaders.end())
		{
			LP_CORE_ERROR("No exporter for asset exists!");
			return;
		}

		if (!asset->IsValid())
		{
			return;
		}

		m_AssetLoaders[asset->GetType()]->Save(asset);
	}

	AssetType AssetManager::GetAssetTypeFromPath(const std::filesystem::path& path)
	{
		return GetAssetTypeFromExtension(path.extension().string());
	}
	AssetType AssetManager::GetAssetTypeFromExtension(const std::string& ext)
	{
		std::string extension = Utils::ToLower(ext);
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
		{
			return AssetType::None;
		}

		return s_AssetExtensionMap.at(extension);
	}
}