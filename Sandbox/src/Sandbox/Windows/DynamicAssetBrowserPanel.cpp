#include "lppch.h"
#include "DynamicAssetBrowserPanel.h"

#include <Lamp/Rendering/Textures/Texture2D.h>
#include <Lamp/Utility/UIUtility.h>

#include <imgui/imgui.h>

using namespace Lamp;

namespace Sandbox
{
	DynamicAssetBrowserPanel::DynamicAssetBrowserPanel(Lamp::AssetType type)
		: EditorWindow("Asset Browser")
	{}

	DynamicAssetBrowserPanel::~DynamicAssetBrowserPanel()
	{}

	void DynamicAssetBrowserPanel::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(DynamicAssetBrowserPanel::OnImGuiUpdate));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(DynamicAssetBrowserPanel::OnUpdateEvent));
	}

	void DynamicAssetBrowserPanel::DrawSearchBar(float height)
	{
		const float buttonSizeOffset = 10.f;
		UI::ScopedColor childColor(ImGuiCol_ChildBg, { 0.2f, 0.2f, 0.2f, 1.f });

		ImGui::BeginChild("##searchBar", { 0.f, height });
		{
			{
				UI::ScopedColor buttonBackground{ ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f} };
				ImGui::Image(UI::GetTextureID(m_searchTexture), { height - buttonSizeOffset, height - buttonSizeOffset }, { 1.f, 1.f }, { 0.f, 0.f } );
			
				ImGui::SameLine();

				ImGui::PushItemWidth(200.f);

				if (UI::InputText("##searchText", m_searchQuery, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (!m_searchQuery.empty())
					{
						m_hasSearchQuery = true;
						Search(m_searchQuery);
					}
					else
					{
						m_hasSearchQuery = false;
					}
				}

				ImGui::PopItemWidth();
			}
		}
		ImGui::EndChild();
	}

	void DynamicAssetBrowserPanel::Search(const std::string& query)
	{
	}

	bool DynamicAssetBrowserPanel::OnImGuiUpdate(Lamp::ImGuiUpdateEvent& e)
	{
		if (!m_isOpen)
		{
			return false;
		}

		ImGui::Begin("Asset Browser", &m_isOpen);
		m_isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		{
			UI::ScopedStyleFloat border{ ImGuiStyleVar_FrameBorderSize, 0.f };

			const float	searchBarHeight = 30.f;
			DrawSearchBar(searchBarHeight);
		}

		ImGui::BeginChild("##view", ImVec2{ ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y });
		{

		}
		ImGui::EndChild();

		ImGui::End();
		return false;
	}

	bool DynamicAssetBrowserPanel::OnUpdateEvent(Lamp::AppUpdateEvent& e)
	{

		return false;
	}

}