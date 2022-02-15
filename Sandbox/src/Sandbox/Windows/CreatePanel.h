#pragma once

#include "Sandbox/Windows/EditorWindow.h"
#include "Sandbox/Windows/AssetBrowserPanel.h"

#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/Core/Core.h>

namespace Lamp
{
	class Object;
	class Texture2D;
}

namespace Sandbox
{
	class CreatePanel : public EditorWindow
	{
	public:
		CreatePanel(Lamp::Object* selectedObject);
		~CreatePanel() override = default;

		void OnEvent(Lamp::Event& e) override;

	private:
		bool OnImGuiRender(Lamp::ImGuiUpdateEvent& e);

		Ref<DirectoryData> ProcessDirectory(const std::filesystem::path& path, Ref<DirectoryData> parent);
		void RenderDirectory(const Ref<DirectoryData> dirData);
		void Reload();
		void Search(const std::string& query);
		void FindAssetsWithQuery(const std::vector<Ref<DirectoryData>>& dirList, const std::string& query);

		bool m_isOpen = true;
		bool m_hasSearchQuery = false;

		Lamp::Object* m_pSelectedObject;
		std::string m_searchQuery;

		Ref<Lamp::Texture2D> m_backTexture;
		Ref<Lamp::Texture2D> m_searchTexture;
		Ref<Lamp::Texture2D> m_reloadTexture;
		Ref<Lamp::Texture2D> m_brushTexture;
		Ref<Lamp::Texture2D> m_directoryTexture;

		std::vector<AssetData> m_searchAssets;
		std::unordered_map<std::string, Ref<DirectoryData>> m_directories;
	};
}