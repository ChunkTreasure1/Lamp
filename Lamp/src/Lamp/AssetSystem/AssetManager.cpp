#include "lppch.h"
#include "AssetManager.h"

#include "Lamp/AssetSystem/LevelLoader.h"
#include "ResourceCache.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"

#include <glm/gtx/quaternion.hpp>

#include <functional>

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
		m_assetLoaders[AssetType::Mesh] = CreateScope<MeshLoader>();
		m_assetLoaders[AssetType::MeshSource] = CreateScope<MeshSourceLoader>();
		m_assetLoaders[AssetType::Texture] = CreateScope<TextureLoader>();
		m_assetLoaders[AssetType::EnvironmentMap] = CreateScope<EnvironmentLoader>();
		m_assetLoaders[AssetType::Material] = CreateScope<MaterialLoader>();
		m_assetLoaders[AssetType::Level] = CreateScope<LevelLoader>();
		LoadAssetRegistry();

		m_loadingAssets.reserve(100);
	}

	void AssetManager::Shutdown()
	{
		LP_PROFILE_FUNCTION();
		SaveAssetRegistry();
	}
	

	void AssetManager::Update()
	{
		//ResourceCache::Update();

		while (!m_loadQueue.empty() && m_threadPool.size() < m_maxThreads)
		{
			std::lock_guard lock{ m_queueMutex };

			auto data = m_loadQueue.front();
			m_loadQueue.pop();

			m_loadingAssets.emplace_back(data);

			auto func = [this](std::reference_wrapper<AssetLoadJob>(data))
			{
				Thread_LoadAsset(data);

				data.get().threadId = std::this_thread::get_id();
				data.get().finished = true;

			};

			m_threadPool.emplace_back(func, std::ref(m_loadingAssets.back()));
		}

		for (int32_t i = m_loadingAssets.size() - 1; i >= 0; --i)
		{
			if (m_loadingAssets[i].finished)
			{
				bool found = false;

				for (int32_t t = m_threadPool.size() - 1; t >= 0; --t)
				{
					if (m_threadPool[t].get_id() == m_loadingAssets[i].threadId)
					{
						m_threadPool[t].join();
						m_threadPool.erase(m_threadPool.begin() + t);

						m_loadingAssets[i].asset->SetFlag(AssetFlag::Unloaded, false);
						m_loadingAssets.erase(m_loadingAssets.begin() + i);

						found = true;

						break;
					}
				}
				
				if (!found)
				{
					LP_CORE_ERROR("[AssetManager]: Asset {0} finished loading, but thread id is invalid!", m_loadingAssets[i].path.string());
				}
			}
		}
				
	}

	void AssetManager::LoadAsset(const std::filesystem::path& path, Asset* asset)
	{
		std::lock_guard lock{ m_queueMutex };

		AssetLoadJob job{path, GetAssetTypeFromPath(path), GetAssetHandleFromPath(path), asset};
		m_loadQueue.push(job);
	}

	void AssetManager::SaveAsset(const Ref<Asset>& asset)
	{
		if (m_assetLoaders.find(asset->GetType()) == m_assetLoaders.end())
		{
			LP_CORE_ERROR("No exporter for asset exists!");
			return;
		}

		if (!asset->IsValid())
		{
			return;
		}

		if (m_assetRegistry.find(asset->Path) == m_assetRegistry.end())
		{
			m_assetRegistry.emplace(asset->Path, asset->Handle);
		}

		m_assetLoaders[asset->GetType()]->Save(asset);
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
		return m_assetRegistry.find(path) != m_assetRegistry.end() ? m_assetRegistry[path] : 0;
	}

	std::filesystem::path AssetManager::GetPathFromAssetHandle(AssetHandle assetHandle)
	{
		for (auto& [path, handle] : m_assetRegistry)
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
		for (const auto& [path, handle] : m_assetRegistry)
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

			m_assetRegistry.emplace(std::make_pair(path, handle));
		}
	}

	void AssetManager::Thread_LoadAsset(AssetLoadJob& loadJob)
	{
		if (m_assetLoaders.find(loadJob.type) == m_assetLoaders.end())
		{
			LP_CORE_ERROR("No importer for asset type {0} found!", loadJob.path.extension().string());
			return;
		}

		auto asset = loadJob.asset;

		m_assetLoaders[loadJob.type]->Load(loadJob.path, asset);

		asset->Path = loadJob.path;
		asset->Handle = loadJob.handle;

		m_assetRegistry[loadJob.path] = asset->Handle;
	}
} 