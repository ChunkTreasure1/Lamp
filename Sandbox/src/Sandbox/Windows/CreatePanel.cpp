#include "CreatePanel.h"

#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/Utility/UIUtility.h>

#include <Lamp/Objects/Entity/Entity.h>

#include <imgui.h>

namespace Sandbox
{
	using namespace Lamp;
	static const std::filesystem::path s_assetsPath = "assets";

	CreatePanel::CreatePanel(Lamp::Object** selectedObject)
		: EditorWindow("Create"), m_pSelectedObject(selectedObject)
	{
		m_backTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/backIcon.png");
		m_searchTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/searchIcon.png");
		m_reloadTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/reloadIcon.png");
		m_brushTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/assetIcons/iconMesh.png");
		m_directoryTexture = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/directoryIcon.png");

		m_directories[s_assetsPath.string()] = ProcessDirectory(s_assetsPath, nullptr);
	}

	void CreatePanel::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(CreatePanel::OnImGuiRender));
	}

	bool CreatePanel::OnImGuiRender(Lamp::ImGuiUpdateEvent& e)
	{
		LP_PROFILE_FUNCTION();
		UI::ScopedColor buttonColor(ImGuiCol_Button, { 0.313f, 0.313f, 0.313f, 1.f });
		UI::ScopedStyleFloat buttonRounding(ImGuiStyleVar_FrameRounding, 2.f);

		if (!m_isOpen)
		{
			return false;
		}

		static bool brushListOpen = false;
		const float buttonHeight = 24.f;

		ImGui::Begin(m_name.c_str(), &m_isOpen);

		m_isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

		if (!brushListOpen)
		{
			const ImGuiTableFlags flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
			if (ImGui::BeginTable("createButtons", 2, flags))
			{
				ImGui::TableSetupColumn("Column1", 0, ImGui::GetContentRegionAvail().x / 2.f);
				ImGui::TableSetupColumn("Column2", 0, ImGui::GetContentRegionAvail().x / 2.f);

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				if (ImGui::Button("Entity", { ImGui::GetContentRegionAvail().x, buttonHeight }))
				{
					Object* obj = *(m_pSelectedObject);

					if (obj)
					{
						obj->SetIsSelected(false);
					}

					(*m_pSelectedObject) = Lamp::Entity::Create();
					obj = *(m_pSelectedObject);
					obj->SetPosition(glm::vec3(0.f, 0.f, 0.f));
					obj->SetIsSelected(true);

					static_cast<Lamp::Entity*>(obj)->SetSaveable(true);
				}

				ImGui::TableNextColumn();

				if (ImGui::Button("Brush", { ImGui::GetContentRegionAvail().x, buttonHeight }))
				{
					brushListOpen = true;
				}

				ImGui::EndTable();
			}
		}
		else
		{
			const float buttonSize = 20.f;

			if (UI::ImageButton("##backButton", UI::GetTextureID(m_backTexture), {buttonSize, buttonSize}))
			{
				brushListOpen = false;
				m_hasSearchQuery = false;
			}

			ImGui::SameLine();

			if (UI::ImageButton("##reloadButton", UI::GetTextureID(m_reloadTexture), {buttonSize, buttonSize}))
			{
				m_hasSearchQuery = false;
				Reload();
			}

			ImGui::SameLine();
			UI::ShiftCursor(0.f, 2.f);
			ImGui::Image(UI::GetTextureID(m_searchTexture), { buttonSize, buttonSize }, { 1, 1 }, { 0, 0 });
			ImGui::SameLine();

			UI::ShiftCursor(0.f, -2.f);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			if (UI::InputText("##searchBar", m_searchQuery, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				if (m_searchQuery.empty())
				{
					m_hasSearchQuery = false;
				}
				else
				{
					Search(m_searchQuery);
					m_hasSearchQuery = true;
				}
			}
			ImGui::PopItemWidth();

			UI::ScopedColor childColor(ImGuiCol_ChildBg, { 0.18f, 0.18f, 0.18f, 1.f });

			UI::PushId();
			if (ImGui::BeginChild("brushes"))
			{
				UI::ShiftCursor(5.f, 5.f);
				if (UI::TreeNodeImage(m_directoryTexture, "Assets", ImGuiTreeNodeFlags_DefaultOpen))
				{
					UI::ScopedStyleFloat2 spacing(ImGuiStyleVar_ItemSpacing, { 0.f, 0.f });

					if (m_hasSearchQuery)
					{
						for (const auto& asset : m_searchAssets)
						{
							std::string assetId = asset.path.stem().string() + "##" + std::to_string(asset.handle);
							ImGui::Selectable(assetId.c_str());
							if (ImGui::BeginDragDropSource())
							{
								const wchar_t* itemPath = asset.path.c_str();
								ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
								ImGui::EndDragDropSource();
							}
						}
					}
					else
					{
						for (const auto& subDir : m_directories[s_assetsPath.string()]->subDirectories)
						{
							RenderDirectory(subDir);
						}
					}

					UI::TreeNodePop();
				}

				ImGui::EndChild();
			}
			UI::PopId();
		}

		ImGui::End();

		return false;
	}

	Ref<DirectoryData> CreatePanel::ProcessDirectory(const std::filesystem::path& path, Ref<DirectoryData> parent)
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

				if (data.type == AssetType::Mesh)
				{
					dirData->assets.push_back(data);
				}
			}
			else
			{
				dirData->subDirectories.emplace_back(ProcessDirectory(entry, dirData));
			}
		}

		for (int i = dirData->subDirectories.size() - 1; i >= 0; --i)
		{
			if (dirData->subDirectories[i]->assets.empty() && dirData->subDirectories[i]->subDirectories.empty())
			{
				dirData->subDirectories.erase(dirData->subDirectories.begin() + i);
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

	void CreatePanel::RenderDirectory(const Ref<DirectoryData> dirData)
	{
		std::string id = dirData->path.stem().string() + "##" + std::to_string(dirData->handle);

		auto flags = (dirData->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None);

		bool open = UI::TreeNodeImage(m_directoryTexture, id.c_str(), flags);

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
				if (ImGui::BeginDragDropSource())
				{
					const wchar_t* itemPath = asset.path.c_str();
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}
			}

			ImGui::TreePop();
		}
	}

	void CreatePanel::Reload()
	{
		m_directories[s_assetsPath.string()] = ProcessDirectory(s_assetsPath, nullptr);
	}

	void CreatePanel::Search(const std::string& query)
	{
		std::vector<std::string> queries;

		std::string searchQuery = query;
		searchQuery.push_back(' ');

		for (auto next = searchQuery.find_first_of(' '); next != std::string::npos; next = searchQuery.find_first_of(' '))
		{
			std::string split = searchQuery.substr(0, next);
			searchQuery = searchQuery.substr(next + 1);

			queries.emplace_back(split);
		}

		//Make queries lower case
		for (auto& query : queries)
		{
			std::transform(query.begin(), query.end(), query.begin(),
				[](unsigned char c) { return std::tolower(c); });
		}

		m_searchAssets.clear();
		for (const auto& query : queries)
		{
			FindAssetsWithQuery(m_directories[s_assetsPath.string()]->subDirectories, m_searchQuery);
		}
	}

	void CreatePanel::FindAssetsWithQuery(const std::vector<Ref<DirectoryData>>& dirList, const std::string& query)
	{
		for (const auto& dir : dirList)
		{
			for (const auto& asset : dir->assets)
			{
				std::string assetStem = asset.path.stem().string();
				std::transform(assetStem.begin(), assetStem.end(), assetStem.begin(), [](unsigned char c)
					{
						return std::tolower(c);
					});

				if (assetStem.find(query) != std::string::npos)
				{
					m_searchAssets.emplace_back(asset);
				}
			}

			FindAssetsWithQuery(dir->subDirectories, query);
		}
	}
}