#include "AssetBrowserPanel.h"

#include <imgui.h>
#include <filesystem>

#include <Lamp/AssetSystem/AssetManager.h>
#include <Lamp/Utility/UIUtility.h>

#include <Lamp/Debug/Instrumentor.h>
#include <Lamp/AssetSystem/ResourceCache.h>

namespace Sandbox
{
	using namespace Lamp;
	static const std::filesystem::path s_assetsPath = "assets";

	AssetBrowserPanel::AssetBrowserPanel()
		: m_currentDirectory(s_assetsPath)
	{
		m_directoryTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/directoryIcon.png");
		m_fileTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/files.png");

		m_icons.emplace(AssetType::Material, ResourceCache::GetAsset<Texture2D>("engine/textures/ui/assetIcons/iconMaterial.png"));
		m_icons.emplace(AssetType::Mesh, ResourceCache::GetAsset<Texture2D>("engine/textures/ui/assetIcons/iconMesh.png"));

		m_directories[s_assetsPath.string()] = ProcessDirectory(s_assetsPath, nullptr);
		m_pCurrentDirectory = m_directories[s_assetsPath.string()].get();
	}

	Ref<DirectoryData> AssetBrowserPanel::ProcessDirectory(const std::filesystem::path& path, Ref<DirectoryData> parent)
	{
		Ref<DirectoryData> dirData = CreateRef<DirectoryData>();
		dirData->path = path;
		dirData->parent = parent.get();

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (!entry.is_directory())
			{
				AssetData data;
				data.path = entry;
				data.handle = g_pEnv->pAssetManager->GetAssetHandleFromPath(entry);
				data.type = g_pEnv->pAssetManager->GetAssetTypeFromPath(entry);

				dirData->assets.push_back(data);
			}
			else
			{
				dirData->subDirectories.push_back(ProcessDirectory(entry, dirData));
			}
		}

		//Sort directories and assets by name
		std::sort(dirData->subDirectories.begin(), dirData->subDirectories.end(), [](const Ref<DirectoryData> dataOne, const Ref<DirectoryData> dataTwo) 
			{
				return dataOne->path.stem().string() < dataTwo->path.stem().string();
			});

		std::sort(dirData->assets.begin(), dirData->assets.end(), [](const AssetData& dataOne, const AssetData& dataTwo)
			{
				return dataOne.path.stem().string() < dataTwo.path.stem().string();
			});

		return dirData;
	}

	void AssetBrowserPanel::RenderControlsBar(float height)
	{
		ImGui::BeginChild("##controlsBar", { 0.f, height });
		{
			ImGui::Button("Test", { height - 2, height - 2 });
		}
		ImGui::EndChild();
	}

	void AssetBrowserPanel::RenderDirectory(const Ref<DirectoryData> dirData)
	{
		std::string id = dirData->path.stem().string() + "##" + std::to_string(dirData->handle);

		auto flags = (dirData->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None) | ImGuiTreeNodeFlags_OpenOnArrow;

		bool open = ImGui::TreeNodeEx(id.c_str(), flags);
		if (ImGui::IsItemClicked())
		{
			dirData->selected = true;
			m_pNextDirectory = dirData.get();
		}

		if (open)
		{
			for (const auto& subDir : dirData->subDirectories)
			{
				RenderDirectory(subDir);
			}

			for (const auto& asset : dirData->assets)
			{
				std::string assetId = asset.path.stem().string() + "##" + std::to_string(asset.handle);
				ImGui::Selectable(assetId.c_str());
			}

			ImGui::TreePop();
		}
	}

	void AssetBrowserPanel::OnImGuiRender()
	{
		if (!m_isOpen)
		{
			return;
		}

		LP_PROFILE_FUNCTION();
		ImGui::Begin("Asset Browser", &m_isOpen);

		static float padding = 16.f;
		static float thumbnailSize = 100.f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
		{
			columnCount = 1;
		}

		const ImGuiTableFlags mainFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable;

		if (m_pNextDirectory)
		{
			m_pCurrentDirectory->selected = false;
			m_pCurrentDirectory = m_pNextDirectory;
			m_pNextDirectory = nullptr;
		}

		UI::PushId();
		if (ImGui::BeginTable("assetBrowserMain", 2, mainFlags))
		{
			ImGui::TableSetupColumn("Outline", 0, 250.f);
			ImGui::TableSetupColumn("View", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			//Draw outline
			ImGui::BeginChild("##outline");
			{
				if (ImGui::CollapsingHeader("Assets", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
				{
					UI::ScopedStyleFloat2 spacing(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });

					for (const auto& subDir : m_directories[s_assetsPath.string()]->subDirectories)
					{
						RenderDirectory(subDir);
					}
				}
			}
			ImGui::EndChild();

			ImGui::TableNextColumn();

			const float controlsBarHeight = 30.f;
			ImGui::BeginChild("##view", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight() - controlsBarHeight));
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
				RenderControlsBar(controlsBarHeight);
				ImGui::PopStyleVar();

				ImGui::Separator();

				ImGui::BeginChild("Scrolling");
				{
					if (m_pCurrentDirectory->path != std::filesystem::path(s_assetsPath))
					{
						if (ImGui::Button("<-"))
						{
							m_pNextDirectory = m_pCurrentDirectory->parent;
							m_pNextDirectory->selected = true;
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

					ImGui::Columns(columnCount, nullptr, false);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f });

					for (auto& dir : m_pCurrentDirectory->subDirectories)
					{
						ImGui::PushID(dir->path.filename().string().c_str());

						ImGui::ImageButton((ImTextureID)m_directoryTexture->GetID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

						if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
						{
							dir->selected = true;
							m_pNextDirectory = dir.get();
						}
						
						ImGui::TextWrapped(dir->path.filename().string().c_str());

						ImGui::NextColumn();
						ImGui::PopID();
					}

					for (const auto& asset : m_pCurrentDirectory->assets)
					{
						ImGui::PushID(asset.path.filename().string().c_str());

						Ref<Texture2D> icon = m_icons[g_pEnv->pAssetManager->GetAssetTypeFromPath(asset.path)];
						if (!icon)
						{
							icon = m_fileTexture;
						}

						ImGui::ImageButton((ImTextureID)icon->GetID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
						if (ImGui::BeginDragDropSource())
						{
							//Data being copied
							const wchar_t* itemPath = asset.path.c_str();
							ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);

							ImGui::EndDragDropSource();
						}

						ImGui::TextWrapped(asset.path.filename().string().c_str());

						ImGui::NextColumn();
						ImGui::PopID();
					}

					ImGui::PopStyleColor();
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();

			ImGui::EndTable();
		}
		UI::PopId();

		ImGui::End();
	}
}
