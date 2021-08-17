#include "AssetManagerPanel.h"

#include <imgui.h>
#include <filesystem>

namespace Sandbox3D
{
	AssetManagerPanel::AssetManagerPanel()
		: m_currentDirectory("assets")
	{

	}

	void AssetManagerPanel::OnImGuiRender()
	{
		ImGui::Begin("Asset Browser", &m_isOpen);

		for (auto& p : std::filesystem::directory_iterator("assets"))
		{
			if (p.is_directory())
			{
				std::string path = p.path().string();
				if (ImGui::Button(path.c_str()))
				{
					m_currentDirectory /= p.path().filename();
				}
			}
		}

		ImGui::End();
	}
}
