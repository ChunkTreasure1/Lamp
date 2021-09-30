#include "lppch.h"
#include "AssetManager.h"

#include "Lamp/Rendering/Texture2D/Texture2D.h"

#include "Lamp/AssetSystem/LevelLoader.h"
#include "Lamp/AssetSystem/RenderGraphLoader.h"
#include "ResourceCache.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"

#include <glm/gtx/quaternion.hpp>


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
		m_AssetLoaders[AssetType::Level] = CreateScope<LevelLoader>();
		LoadAssetRegistry();
	}

	void AssetManager::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		SaveAssetRegistry();
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

	AssetHandle AssetManager::GetAssetHandleFromPath(const std::filesystem::path& path)
	{
		return m_AssetRegistry.find(path) != m_AssetRegistry.end() ? m_AssetRegistry[path] : 0;
	}

	std::filesystem::path AssetManager::GetPathFromAssetHandle(AssetHandle assetHandle)
	{
		for (auto& [path, handle] : m_AssetRegistry)
		{
			if (handle == assetHandle)
			{
				return path;
			}
		}

		return "";
	}

	void AssetManager::SaveAssetRegistry()
	{
		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "Assets" << YAML::BeginSeq;
		for (const auto& [path, handle] : m_AssetRegistry)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << YAML::Value << handle;
			out << YAML::Key << "Path" << YAML::Value << path.string();
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		std::ofstream fout("assets/assetRegistry.lpreg");
		fout << out.c_str();
		fout.close();
	}

	void AssetManager::LoadAssetRegistry()
	{
		if (!std::filesystem::exists("assets/assetRegistry.lpreg"))
		{
			return;
		}

		std::ifstream stream("assets/assetRegistry.lpreg");

		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node root = YAML::Load(strStream.str());
		YAML::Node assets = root["Assets"];

		for (auto entry : assets)
		{
			std::string path = entry["Path"].as<std::string>();
			AssetHandle handle = entry["Handle"].as<AssetHandle>();

			m_AssetRegistry.emplace(std::make_pair(path, handle));
		}
	}
}