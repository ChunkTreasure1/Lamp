#include "EnvironmentEditorPanel.h"

#include <Lamp/Utility/UIUtility.h>
#include <Lamp/Core/Time/ScopedTimer.h>

namespace Sandbox
{
	using namespace Lamp;

	EnvironmentEditorPanel::EnvironmentEditorPanel(std::string_view name)
		: BaseWindow(name)
	{
	}

	void EnvironmentEditorPanel::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(EnvironmentEditorPanel::OnImGuiUpdate));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(EnvironmentEditorPanel::OnUpdate));
	}

	bool EnvironmentEditorPanel::OnImGuiUpdate(Lamp::ImGuiUpdateEvent& e)
	{
		LP_PROFILE_FUNCTION();

		ScopedTimer timer{};

		if (!m_IsOpen)
		{
			return false;
		}

		ImGui::Begin("Environment Editor", &m_IsOpen);

		if (!LevelManager::GetActive())
		{
			ImGui::TextUnformatted("No level loaded!");
		}
		else
		{
			if (UI::TreeNode("Skybox", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& skyboxData = const_cast<SkyboxData&>(LevelManager::GetActive()->GetEnvironment().GetSkybox());

				if (UI::BeginProperties())
				{
					UI::Property("LOD", skyboxData.environmentLod);
					UI::Property("Light Multiplier", skyboxData.environmentMultiplier);

					static std::filesystem::path skyboxPath;

					if (skyboxData.skybox)
					{
						skyboxPath = skyboxData.skybox->Path;
					}

					if (UI::Property("Texture", skyboxPath))
					{
						if (std::filesystem::exists(skyboxPath))
						{
							skyboxData.skybox = Skybox::Create(skyboxPath);
						}
					}

					UI::EndProperties();
				}

				UI::TreeNodePop();
			}

			if (UI::TreeNode("Terrain", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& terrainData = const_cast<TerrainData&>(LevelManager::GetActive()->GetEnvironment().GetTerrain());

				if (UI::BeginProperties())
				{
					UI::Property("Scale", terrainData.terrainScale);
					UI::Property("Shift", terrainData.terrainShift);

					static std::filesystem::path heightMapPath;
					if (terrainData.terrain && terrainData.terrain->GetHeightMap())
					{
						heightMapPath = terrainData.terrain->GetHeightMap()->Path;
					}

					if (UI::Property("Height Map", heightMapPath))
					{
						if (std::filesystem::exists(heightMapPath))
						{
							terrainData.terrain = Terrain::Create(heightMapPath);
							terrainData.terrain->SetupRenderPipeline(LevelManager::GetActive()->GetGeometryFramebuffer());
						}
					}

					UI::EndProperties();
				}

				UI::TreeNodePop();
			}
		}

		ImGui::End();

		m_deltaTime = timer.GetTime();

		return false;
	}

	bool EnvironmentEditorPanel::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		return false;
	}
}