#pragma once

#include <filesystem>
#include "Lamp/Rendering/Texture2D/Texture2D.h"

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

	class AssetBrowserPanel
	{
	public:
		AssetBrowserPanel();

		Ref<DirectoryData> ProcessDirectory(const std::filesystem::path& path, Ref<DirectoryData> parent);
		
		void RenderControlsBar(float height);
		void RenderDirectory(const Ref<DirectoryData> dirData);
		void OnImGuiRender();

		bool& GetIsOpen() { return m_isOpen; }

	private:
		bool m_isOpen = true;
		std::filesystem::path m_currentDirectory;
		Ref<Lamp::Texture2D> m_directoryTexture;
		Ref<Lamp::Texture2D> m_fileTexture;
		std::unordered_map<Lamp::AssetType, Ref<Lamp::Texture2D>> m_icons;

		std::unordered_map <std::string, Ref<DirectoryData>> m_directories;
		std::vector<Lamp::AssetHandle> m_selectedAssets;

		DirectoryData* m_pCurrentDirectory = nullptr;
		DirectoryData* m_pNextDirectory = nullptr;
	};
}