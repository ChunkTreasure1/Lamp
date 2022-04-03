#include "TerrainEditorPanel.h"

#include <Lamp/Rendering/Textures/Texture2D.h>

#include <Lamp/Input/Input.h>
#include <Lamp/Input/KeyCodes.h>

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
		dispatcher.Dispatch<KeyPressedEvent>(LP_BIND_EVENT_FN(TerrainEditorPanel::OnKeyPressedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(LP_BIND_EVENT_FN(TerrainEditorPanel::OnMouseScrolledEvent));
	}

	bool TerrainEditorPanel::OnUpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		LP_PROFILE_FUNCTION();
		UI::ScopedColor buttonColor(ImGuiCol_Button, { 0.313f, 0.313f, 0.313f, 1.f });
		UI::ScopedStyleFloat buttonRounding(ImGuiStyleVar_FrameRounding, 2.f);

		ImGui::Begin("Terrain Editor", &m_isOpen);
		m_isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		if (!LevelManager::IsLevelLoaded())
		{
			ImGui::End();
			return false;
		}

		const ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
		const float buttonHeight = 24.f;

		if (ImGui::BeginTable("paintButtons", 2, flags))
		{
			ImGui::TableSetupColumn("Column1", 0, ImGui::GetContentRegionAvail().x / 2.f);
			ImGui::TableSetupColumn("Column2", 0, ImGui::GetContentRegionAvail().x / 2.f);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			if (ImGui::Button("Flatten", { ImGui::GetContentRegionAvail().x, buttonHeight }))
			{
				if (m_editParameters.currentEditMode == EditMode::Flatten)
				{
					m_editParameters.isEditing = !m_editParameters.isEditing;
				}

				m_editParameters.currentEditMode = EditMode::Flatten;
			}

			ImGui::TableNextColumn();

			if (ImGui::Button("Raise/Lower", { ImGui::GetContentRegionAvail().x, buttonHeight }))
			{
				if (m_editParameters.currentEditMode == EditMode::RaiseLower)
				{
					m_editParameters.isEditing = !m_editParameters.isEditing;
				}

				m_editParameters.currentEditMode = EditMode::RaiseLower;
			}

			ImGui::TableNextRow();

			if (ImGui::Button("Smooth", { ImGui::GetContentRegionAvail().x, buttonHeight }))
			{
				if (m_editParameters.currentEditMode == EditMode::Smooth)
				{
					m_editParameters.isEditing = !m_editParameters.isEditing;
				}

				m_editParameters.currentEditMode = EditMode::Smooth;
			}

			ImGui::TableNextColumn();

			if (ImGui::Button("Spline", { ImGui::GetContentRegionAvail().x, buttonHeight }))
			{
				//Create new terrain spline
			}

			ImGui::EndTable();
		}

		if (m_editParameters.isEditing)
		{
			ImGui::PushID("brushProperties");
			UI::BeginProperties("brushProperties", false);

			UI::Property("Inner Radius", m_editParameters.innerRadius);
			UI::Property("Outer Radius", m_editParameters.outerRadius);
			UI::Property("Hardness", m_editParameters.hardness, 0.f, 1.f);

			UI::EndProperties(false);
			ImGui::PopID();
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
					Ref<Texture2D> terrainTex = Texture2D::Create(ImageFormat::RGBA, heightMapRes, heightMapRes, true);

					auto data = GenerateEmptyTextureData(heightMapRes);
					terrainTex->SetData(data.data(), (uint32_t)(data.size() * sizeof(uint32_t)));

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

	bool TerrainEditorPanel::OnKeyPressedEvent(Lamp::KeyPressedEvent& e)
	{
		if (m_editParameters.isEditing)
		{
			return false;
		}

		return false;
	}

	bool TerrainEditorPanel::OnRenderEvent(Lamp::AppRenderEvent& e)
	{


		return false;
	}

	bool TerrainEditorPanel::OnMouseScrolledEvent(Lamp::MouseScrolledEvent& e)
	{
		if (!m_editParameters.isEditing)
		{
			if (Input::IsKeyPressed(LP_KEY_LEFT_ALT))
			{
				m_editParameters.outerRadius += e.GetYOffset() * m_scrollSpeedMultiplier;
			}

			return false;
		}

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