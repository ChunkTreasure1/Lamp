#include "AssetBrowserPanel.h"

#include <imgui.h>
#include <filesystem>

#include <Lamp/AssetSystem/AssetManager.h>
#include <Lamp/Utility/UIUtility.h>
#include <Lamp/AssetSystem/BaseAssets.h>
#include <Lamp/Mesh/Mesh.h>
#include <Lamp/Rendering/Textures/Texture2D.h>

#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/Input/Input.h>
#include <Lamp/Input/KeyCodes.h>

namespace Sandbox
{
	using namespace Lamp;
	static const std::filesystem::path s_assetsPath = "assets";

	AssetBrowserPanel::AssetBrowserPanel()
		: EditorWindow("Asset Browser"), m_currentDirectory(s_assetsPath)
	{
		m_directoryTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/directoryIcon.png");
		m_fileTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/files.png");
		m_reloadTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/reloadIcon.png");
		m_backTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/backIcon.png");
		m_settingsTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/settingsIcon.png");
		m_searchTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/searchIcon.png");

		m_icons.emplace(AssetType::Material, ResourceCache::GetAsset<Texture2D>("engine/textures/ui/assetIcons/iconMaterial.png"));
		m_icons.emplace(AssetType::Mesh, ResourceCache::GetAsset<Texture2D>("engine/textures/ui/assetIcons/iconMesh.png"));
		m_icons.emplace(AssetType::MeshSource, ResourceCache::GetAsset<Texture2D>("engine/textures/ui/assetIcons/iconMeshSource.png"));

		m_directories[s_assetsPath.string()] = ProcessDirectory(s_assetsPath, nullptr);

		m_pCurrentDirectory = m_directories[s_assetsPath.string()].get();
		m_directoryButtons.emplace_back(m_pCurrentDirectory);
	}

	void AssetBrowserPanel::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(AssetBrowserPanel::OnImGuiRender));
	}

	bool AssetBrowserPanel::OnImGuiRender(Lamp::ImGuiUpdateEvent& e)
	{
		if (!m_isOpen)
		{
			return false;
		}

		LP_PROFILE_FUNCTION();
		ImGui::Begin("Asset Browser", &m_isOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		m_isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		static float padding = 16.f;
		static float thumbnailSize = 100.f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;

		const ImGuiTableFlags mainFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable;

		if (m_pNextDirectory)
		{
			m_pCurrentDirectory->selected = false;
			m_pCurrentDirectory = m_pNextDirectory;
			m_pNextDirectory = nullptr;

			m_directoryButtons.clear();
			m_directoryButtons = FindParentDirectoriesOfDirectory(m_pCurrentDirectory);
		}

		UI::PushId();

		const float controlsBarHeight = 30.f;

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
		DrawControlsBar(controlsBarHeight);
		ImGui::PopStyleVar();

		if (ImGui::BeginTable("assetBrowserMain", 2, mainFlags))
		{
			ImGui::TableSetupColumn("Outline", 0, 250.f);
			ImGui::TableSetupColumn("View", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			//Draw outline
			{
				ImGuiStyle& style = ImGui::GetStyle();
				auto color = style.Colors[ImGuiCol_FrameBg];

				UI::ScopedColor newColor(ImGuiCol_ChildBg, { color.x, color.y, color.z, color.w });
				ImGui::BeginChild("##outline");
			}

			{
				UI::ShiftCursor(5.f, 5.f);
				if (UI::TreeNodeImage(m_directoryTexture, "Assets", ImGuiTreeNodeFlags_DefaultOpen))
				{
					UI::ScopedStyleFloat2 spacing(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });

					for (const auto& subDir : m_directories[s_assetsPath.string()]->subDirectories)
					{
						RenderDirectory(subDir);
					}
					UI::TreeNodePop();
				}
			}
			ImGui::EndChild();

			ImGui::TableNextColumn();

			ImGui::BeginChild("##view", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight() - controlsBarHeight));
			{
				ImGui::BeginChild("Scrolling");
				{
					static float padding = 16.f;

					float cellSize = m_thumbnailSize + padding;
					float panelWidth = ImGui::GetContentRegionAvail().x;
					int columnCount = (int)(panelWidth / cellSize);

					if (columnCount < 1)
					{
						columnCount = 1;
					}

					ImGui::Columns(columnCount, nullptr, false);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.f, 0.f, 0.f, 0.f });

					if (!m_hasSearchQuery)
					{
						RenderView(m_pCurrentDirectory->subDirectories, m_pCurrentDirectory->assets);
					}
					else
					{
						RenderView(m_searchDirectories, m_searchAssets);
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

		return false;
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

				if (data.type != Lamp::AssetType::None)
				{
					dirData->assets.push_back(data);
				}
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

	void AssetBrowserPanel::DrawControlsBar(float height)
	{
		UI::ScopedColor childColor(ImGuiCol_ChildBg, { 0.2f, 0.2f, 0.2f, 1.f });

		ImGui::BeginChild("##controlsBar", { 0.f, height });
		{
			const float buttonSizeOffset = 10.f;
			int offsetToRemove = 0;
			bool shouldRemove = false;

			UI::ShiftCursor(5.f, 4.f);

			{
				UI::ScopedColor buttonBackground(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
				ImGui::Image(UI::GetTextureID(m_searchTexture), { height - buttonSizeOffset, height - buttonSizeOffset }, { 1, 1 }, { 0, 0 });

				ImGui::SameLine();
				UI::ShiftCursor(0.f, -0.5f);

				ImGui::PushItemWidth(200.f);

				if (UI::InputText("##searchBar", m_searchQuery, ImGuiInputTextFlags_EnterReturnsTrue))
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

			ImGui::SameLine();
			UI::ShiftCursor(0.f, -1.f);
			{
				UI::ScopedColor buttonBackground(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });


				if (UI::ImageButton("##reloadButton", UI::GetTextureID(m_reloadTexture), {height - buttonSizeOffset, height - buttonSizeOffset}))
				{
					Reload();
				}

				ImGui::SameLine();

				if (UI::ImageButton("##backButton", UI::GetTextureID(m_backTexture), {height - buttonSizeOffset, height - buttonSizeOffset}))
				{
					if (m_pCurrentDirectory->path != std::filesystem::path(s_assetsPath))
					{
						m_pNextDirectory = m_pCurrentDirectory->parent;
						
						m_hasSearchQuery = false;
						m_pNextDirectory->selected = true;

						offsetToRemove = (uint32_t)(m_directoryButtons.size() - 1);
						shouldRemove = true;
					}
				}
			}

			for (uint32_t i = 0; i < m_directoryButtons.size(); i++)
			{
				ImGui::SameLine();

				std::string dirName = m_directoryButtons[i]->path.stem().string();

				float width = ImGui::CalcTextSize(dirName.c_str()).x + 10.f;

				if (ImGui::Button(dirName.c_str(), { width, height - 4.f }))
				{
					m_pNextDirectory = m_directoryButtons[i];
					m_pNextDirectory->selected = true;

					offsetToRemove = i + 1;
					m_hasSearchQuery = false;
					shouldRemove = true;
				}
			}

			ImGui::SameLine();

			{
				UI::ScopedColor buttonBackground(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });

				UI::ShiftCursor(ImGui::GetContentRegionAvail().x - height - buttonSizeOffset / 2.f, 0.f);
				if (ImGui::ImageButton(UI::GetTextureID(m_settingsTexture), { height - buttonSizeOffset, height - buttonSizeOffset }))
				{
				}
			}

			if (ImGui::BeginPopupContextItem("settingsMenu", ImGuiPopupFlags_MouseButtonLeft))
			{
				ImGui::PushItemWidth(100.f);
				ImGui::SliderFloat("Icon size", &m_thumbnailSize, 20.f, 200.f);
				ImGui::PopItemWidth();

				ImGui::EndPopup();
			}

			if (shouldRemove)
			{
				for (size_t i = m_directoryButtons.size() - 1; i >= offsetToRemove; i--)
				{
					m_directoryButtons.erase(m_directoryButtons.begin() + i);
				}
			}
		}
		ImGui::EndChild();
	}

	void AssetBrowserPanel::RenderDirectory(const Ref<DirectoryData> dirData)
	{
		std::string id = dirData->path.stem().string() + "##" + std::to_string(dirData->handle);

		auto flags = (dirData->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None) | ImGuiTreeNodeFlags_OpenOnArrow;

		bool open = UI::TreeNodeImage(m_directoryTexture, id.c_str(), flags);
		if (ImGui::IsItemClicked())
		{
			dirData->selected = true;
			m_hasSearchQuery = false;
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

	void AssetBrowserPanel::Reload()
	{
		const std::filesystem::path currentPath = m_pCurrentDirectory ? m_pCurrentDirectory->path : s_assetsPath;

		m_pCurrentDirectory = nullptr;
		m_pNextDirectory = nullptr;

		m_directories[s_assetsPath.string()] = ProcessDirectory(s_assetsPath.string(), nullptr);

		//Find directory
		m_pCurrentDirectory = FindDirectoryWithPath(currentPath);
		if (!m_pCurrentDirectory)
		{
			m_pCurrentDirectory = m_directories[s_assetsPath.string()].get();
		}

		//Setup new file path buttons
		m_directoryButtons.clear();
		m_directoryButtons = FindParentDirectoriesOfDirectory(m_pCurrentDirectory);
	}

	void AssetBrowserPanel::Search(const std::string& query)
	{
		std::vector<std::string> queries;
		std::vector<std::string> types;

		std::string searchQuery = query;

		searchQuery.push_back(' ');

		for (auto next = searchQuery.find_first_of(' '); next != std::string::npos; next = searchQuery.find_first_of(' '))
		{
			std::string split = searchQuery.substr(0, next);
			searchQuery = searchQuery.substr(next + 1);

			if (split.front() == '*')
			{
				types.emplace_back(split);
			}
			else
			{
				queries.emplace_back(split);
			}
		}

		//Find all folders and files containing queries
		m_searchDirectories.clear();
		m_searchAssets.clear();
		for (const auto& query : queries)
		{
			FindFoldersAndFilesWithQuery(m_directories[s_assetsPath.string()]->subDirectories, m_searchDirectories, m_searchAssets, query);
		}
	}

	void AssetBrowserPanel::RenderFilePopup(const AssetData& data)
	{
		if (UI::BeginPopup())
		{
			if (ImGui::MenuItem("Reimport", "Ctrl + R"))
			{
				ReimportAsset(data);
			}


			UI::EndPopup();
		}
	}

	void AssetBrowserPanel::ReimportAsset(const AssetData& data)
	{
		switch (data.type)
		{
			case Lamp::AssetType::MeshSource:
			{
				//Get asset
				Ref<MeshSource> meshSource = ResourceCache::GetAsset<MeshSource>(data.path);
				Ref<Asset>& asset = ResourceCache::s_assetCache[g_pEnv->pAssetManager->GetPathFromAssetHandle(meshSource->GetMeshAsset())];
				AssetHandle oldHandle = data.handle;

				asset = MeshImporter::ImportMesh(meshSource->GetImportSettings());
				asset->Handle = oldHandle;

				g_pEnv->pAssetManager->SaveAsset(asset);

				break;
			}
		}
	}

	void AssetBrowserPanel::RenderView(const std::vector<Ref<DirectoryData>>& dirData, const std::vector<AssetData>& assetData)
	{
		for (auto& dir : dirData)
		{
			ImGui::PushID(dir->path.filename().string().c_str());

			UI::ImageButton(dir->path.filename().string(), UI::GetTextureID(m_directoryTexture), { m_thumbnailSize, m_thumbnailSize }, { 0, 1 }, { 1, 0 });
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
			{
				dir->selected = true;
				m_pNextDirectory = dir.get();
			}

			ImGui::TextWrapped(dir->path.filename().string().c_str());

			ImGui::NextColumn();
			ImGui::PopID();
		}

		for (const auto& asset : assetData)
		{
			ImGui::PushID(asset.path.filename().string().c_str());

			Ref<Texture2D> icon = m_icons[g_pEnv->pAssetManager->GetAssetTypeFromPath(asset.path)];
			if (!icon)
			{
				icon = m_fileTexture;
			}

			UI::ImageButton(asset.path.filename().string(), UI::GetTextureID(icon), { m_thumbnailSize, m_thumbnailSize }, { 0, 1 }, { 1, 0 });
			if (ImGui::BeginDragDropSource())
			{
				//Data being copied
				const wchar_t* itemPath = asset.path.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);

				ImGui::EndDragDropSource();
			}
			RenderFilePopup(asset);

			ImGui::TextWrapped(asset.path.filename().string().c_str());

			ImGui::NextColumn();
			ImGui::PopID();
		}
	}

	void AssetBrowserPanel::FindFoldersAndFilesWithQuery(const std::vector<Ref<DirectoryData>>& dirList, std::vector<Ref<DirectoryData>>& directories, std::vector<AssetData>& assets, const std::string& query)
	{
		for (const auto& dir : dirList)
		{
			std::string dirStem = dir->path.stem().string();
			std::transform(dirStem.begin(), dirStem.end(), dirStem.begin(), [](unsigned char c) 
				{
					return std::tolower(c);
				});

			if (dirStem.find(query) != std::string::npos)
			{
				directories.emplace_back(dir);
			}

			for (const auto& asset : dir->assets)
			{
				std::string assetStem = asset.path.stem().string();
				std::transform(assetStem.begin(), assetStem.end(), assetStem.begin(), [](unsigned char c)
					{
						return std::tolower(c);
					});

				if (assetStem.find(query) != std::string::npos)
				{
					assets.emplace_back(asset);
				}
			}

			FindFoldersAndFilesWithQuery(dir->subDirectories, directories, assets, query);
		}
	}

	std::vector<DirectoryData*> AssetBrowserPanel::FindParentDirectoriesOfDirectory(DirectoryData* directory)
	{
		std::vector<DirectoryData*> directories;
		directories.emplace_back(directory);
		for (auto dir = directory->parent; dir != nullptr; dir = dir->parent)
		{
			directories.emplace_back(dir);
		}

		std::reverse(directories.begin(), directories.end());

		return directories;
	}

	DirectoryData* AssetBrowserPanel::FindDirectoryWithPath(const std::filesystem::path& path)
	{
		std::vector<Ref<DirectoryData>> dirList;
		for (const auto& dir : m_directories)
		{
			dirList.emplace_back(dir.second);
		}

		return FindDirectoryWithPathRecursivly(dirList, path);
	}

	DirectoryData* AssetBrowserPanel::FindDirectoryWithPathRecursivly(const std::vector<Ref<DirectoryData>> dirList, const std::filesystem::path& path)
	{
		for (const auto& dir : dirList)
		{
			if (dir->path == path)
			{
				return dir.get();
			}
		}

		for (const auto& dir : dirList)
		{
			if (auto it = FindDirectoryWithPathRecursivly(dir->subDirectories, path))
			{
				return it;
			}
		}

		return nullptr;
	}
}
