#include "AssetManagerPanel.h"

#include <imgui.h>
#include <filesystem>

#include <Lamp/AssetSystem/ResourceCache.h>

namespace Sandbox3D
{
	using namespace Lamp;
	static const std::filesystem::path s_assetsPath = "assets";

	AssetManagerPanel::AssetManagerPanel()
		: m_currentDirectory(s_assetsPath)
	{
		m_directoryTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/directoryIcon.png");
		m_fileTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/files.png");

		m_Icons.emplace(std::make_pair(AssetType::Material, ResourceCache::GetAsset<Texture2D>("engine/textures/ui/assetIcons/iconMaterial.png")));
		m_Icons.emplace(std::make_pair(AssetType::Mesh, ResourceCache::GetAsset<Texture2D>("engine/textures/ui/assetIcons/iconMesh.png")));
	}

	void AssetManagerPanel::OnImGuiRender()
	{
		LP_PROFILE_FUNCTION();
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

			Ref<Lamp::Texture2D> icon;
			if (directoryEntry.is_directory())
			{
				icon = m_directoryTexture;
			}
			else
			{
				AssetType type = g_pEnv->pAssetManager->GetAssetTypeFromPath(path);
				if (m_Icons.find(type) != m_Icons.end())
				{
					icon = m_Icons[type];
				}
				else
				{
					icon = m_fileTexture;
				}
			}

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
					//Open asset viewer of editor
				}
			}

			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();
			ImGui::PopID();
		}

		ImGui::Columns(1);

		ImGui::End();
	}
}
