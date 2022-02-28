#pragma once

#include "EditorWindow.h"

#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/AssetSystem/Asset.h>

#include <filesystem>

namespace Lamp
{
	class Texture2D;
}

namespace Sandbox
{
	struct AssetData
	{
		Lamp::AssetHandle handle;
		Lamp::AssetType type;
		std::filesystem::path path;
	};

	struct DirectoryData
	{
		Lamp::AssetHandle handle;
		std::filesystem::path path;
		DirectoryData* parent;
		bool selected = false;

		std::vector<AssetData> assets;
		std::vector<Ref<DirectoryData>> subDirectories;
	};

	class AssetBrowserPanel : public EditorWindow
	{
	public:
		AssetBrowserPanel();
		~AssetBrowserPanel() override = default;

		void OnEvent(Lamp::Event & e) override;

	private:
		bool OnImGuiRender(Lamp::ImGuiUpdateEvent& e);

		Ref<DirectoryData> ProcessDirectory(const std::filesystem::path& path, Ref<DirectoryData> parent);
		
		void DrawControlsBar(float height);
		void RenderDirectory(const Ref<DirectoryData> dirData);
		void Reload();
		
		void Search(const std::string& query);
		void RenderFilePopup(const AssetData& asset);
		void ReimportAsset(const AssetData& asset);

		void RenderView(const std::vector<Ref<DirectoryData>>& dirData, const std::vector<AssetData>& assetData);
		void FindFoldersAndFilesWithQuery(const std::vector<Ref<DirectoryData>>& dirList, std::vector<Ref<DirectoryData>>& directories, std::vector<AssetData>& assets, const std::string& query);

		std::vector<DirectoryData*> FindParentDirectoriesOfDirectory(DirectoryData* directory);

		DirectoryData* FindDirectoryWithPath(const std::filesystem::path& path);
		DirectoryData* FindDirectoryWithPathRecursivly(const std::vector<Ref<DirectoryData>> dirList, const std::filesystem::path& path);

	private:
		bool m_isOpen = true;
		bool m_hasSearchQuery = false;

		float m_thumbnailSize = 80.f;

		std::filesystem::path m_currentDirectory;
		std::string m_searchQuery;

		Ref<Lamp::Texture2D> m_directoryTexture;
		Ref<Lamp::Texture2D> m_fileTexture;
		Ref<Lamp::Texture2D> m_reloadTexture;
		Ref<Lamp::Texture2D> m_backTexture;
		Ref<Lamp::Texture2D> m_settingsTexture;
		Ref<Lamp::Texture2D> m_searchTexture;
		
		std::unordered_map<Lamp::AssetType, Ref<Lamp::Texture2D>> m_icons;
		std::unordered_map <std::string, Ref<DirectoryData>> m_directories;
		std::vector<Lamp::AssetHandle> m_selectedAssets;
		std::vector<DirectoryData*> m_directoryButtons;

		std::vector<Ref<DirectoryData>> m_searchDirectories;
		std::vector<AssetData> m_searchAssets;

		DirectoryData* m_pCurrentDirectory = nullptr;
		DirectoryData* m_pNextDirectory = nullptr;
	};
}