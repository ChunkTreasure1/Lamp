#pragma once

#include "Sandbox/Windows/EditorWindow.h"

#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/AssetSystem/Asset.h>

namespace Lamp
{
	class Texture2D;
}

namespace Sandbox
{
	class DynamicAssetBrowserPanel : public EditorWindow
	{
	public:
		DynamicAssetBrowserPanel(Lamp::AssetType type);
		~DynamicAssetBrowserPanel() override;

		void OnEvent(Lamp::Event& e) override;

	private:
		void DrawSearchBar(float height);
		void Search(const std::string& query);

		bool OnImGuiUpdate(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdateEvent(Lamp::AppUpdateEvent& e);

		Ref<Lamp::Texture2D> m_searchTexture;
		
		std::string m_searchQuery;
		bool m_hasSearchQuery = false;
	};
}