#include "AssetManagerPanel.h"

#include <imgui.h>
#include <filesystem>

namespace Sandbox3D
{
	static const std::filesystem::path s_assetsPath = "assets";

	AssetManagerPanel::AssetManagerPanel()
		: m_currentDirectory(s_assetsPath)
	{
		m_directoryTexture = Lamp::Texture2D::Create("engine/textures/ui/directoryIcon.png");
		m_fileTexture = Lamp::Texture2D::Create("engine/textures/ui/files.png");
	}

	void AssetManagerPanel::OnImGuiRender()
	{
		ImGui::Begin("Asset Browser", &m_isOpen);

		if (m_currentDirectory != std::filesystem::path(s_assetsPath))
		{
			if (ImGui::Button("<-"))
			{
				m_currentDirectory = m_currentDirectory.parent_path();
			}
		}

		static float padding = 16.f;
		static float thumbnailSize = 100.f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
		{
			columnCount = 1;
		}

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_currentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, s_assetsPath);
			std::string filenameString = relativePath.filename().string();

			ImGui::PushID(filenameString.c_str());
			
			Ref<Lamp::Texture2D> icon = directoryEntry.is_directory() ? m_directoryTexture : m_fileTexture;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f });
			
			ImGui::ImageButton((ImTextureID)icon->GetID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
			if (ImGui::BeginDragDropSource())
			{
				//Data being copied
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);

				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
			{
				if (directoryEntry.is_directory())
				{
					m_currentDirectory /= path.filename();
				}
				else
				{
				}
			}

			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();
			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16.f, 512.f);
		ImGui::SliderFloat("Padding", &padding, 0.f, 32.f);

		ImGui::End();
	}
}
