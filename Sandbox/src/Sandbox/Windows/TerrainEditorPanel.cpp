#include "TerrainEditorPanel.h"

#include <Lamp/Rendering/Textures/Texture2D.h>

#include <Lamp/Level/LevelManager.h>
#include <Lamp/Utility/UIUtility.h>

#include <imgui.h>

namespace Sandbox
{
	using namespace Lamp;

	TerrainEditorPanel::TerrainEditorPanel(std::string_view name)
		: EditorWindow(name)
	{
	}

	void TerrainEditorPanel::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(TerrainEditorPanel::OnUpdateImGui));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(TerrainEditorPanel::OnUpdate));
	}

	bool TerrainEditorPanel::OnUpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		ImGui::Begin("Terrain Editor", &m_isOpen);
		m_isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		if (!LevelManager::IsLevelLoaded())
		{
			ImGui::End();
			return false;
		}

		if (!LevelManager::GetActive()->HasTerrain())
		{
			if (ImGui::Button("Create Terrain"))
			{
				ImGui::OpenPopup("New Terrain");
			}

			if (ImGui::BeginPopupModal("New Terrain", nullptr))
			{
				ImGui::Text("New Terrain");
				ImGui::Separator();

				if (UI::BeginProperties())
				{
					UI::Combo("Height map resolution", m_selectedResolution, m_resolutions, 200.f);
					UI::Combo("Meters per pixel", m_selectedSize, m_perPixelSize, 200.f);

					UI::EndProperties();
				}

				ImGui::Text("%s%d", "Total terrain size: ", CalculateTerrainSize());
				ImGui::Separator();

				if (ImGui::Button("Create"))
				{
					const uint32_t heightMapRes = GetHeightMapResolution();
					Ref<Texture2D> terrainTex = Texture2D::Create(ImageFormat::RGBA, heightMapRes, heightMapRes);
					
					auto data = GenerateEmptyTextureData(heightMapRes);
					terrainTex->SetData(data.data(), data.size() * sizeof(uint32_t));

					auto& terrain = const_cast<TerrainData&>(LevelManager::GetActive()->GetEnvironment().GetTerrain());
					terrain.terrain = Terrain::Create(terrainTex);

					ImGui::CloseCurrentPopup();
				}
				ImGui::SetItemDefaultFocus();

				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::End();

		return false;
	}

	bool TerrainEditorPanel::OnUpdate(Lamp::AppUpdateEvent& e)
	{


		return false;
	}

	std::vector<uint32_t> TerrainEditorPanel::GenerateEmptyTextureData(uint32_t resolution)
	{
		std::vector<uint32_t> values;
		values.resize(resolution * resolution);

		std::fill(values.begin(), values.end(), 0xff000000);

		return values;
	}

	uint32_t TerrainEditorPanel::CalculateTerrainSize()
	{
		uint32_t terrainSize = (m_minResolution * (m_selectedResolution + 1)) * (m_minSize * (m_selectedSize + 1));
		return terrainSize;
	}

	uint32_t TerrainEditorPanel::GetHeightMapResolution()
	{
		return (m_minResolution * (m_selectedResolution + 1));
	}

	float TerrainEditorPanel::GetMetersPerPixel()
	{
		return (m_minSize * (m_selectedSize + 1));
	}
}