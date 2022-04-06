#include "MaterialEditorPanel.h"

#include <Lamp/Mesh/Materials/MaterialLibrary.h>
#include <Lamp/Mesh/Mesh.h>
#include <Lamp/Mesh/Materials/MaterialInstance.h>

#include <Lamp/AssetSystem/AssetManager.h>
#include <Lamp/AssetSystem/ResourceCache.h>

#include <Lamp/Rendering/RenderCommand.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Rendering/Cameras/PerspectiveCameraController.h>
#include <Lamp/Rendering/RenderPipelineLibrary.h>
#include <Lamp/Rendering/Buffers/Framebuffer.h>
#include <Lamp/Rendering/Renderer.h>

#include <Lamp/Core/Time/ScopedTimer.h>

#include <Lamp/Utility/UIUtility.h>
#include <Lamp/Utility/PlatformUtility.h>

#include <Lamp/Input/KeyCodes.h>
#include <Lamp/Input/Input.h>

#include <imgui/imgui_stdlib.h>

namespace Sandbox
{
	using namespace Lamp;
	static const std::filesystem::path s_assetsPath = "assets";

	MaterialEditorPanel::MaterialEditorPanel(std::string_view name)
		: EditorWindow(name)
	{
		m_renderPipeline = RenderPipelineLibrary::GetPipeline("Forward");
		m_framebuffer = m_renderPipeline->GetSpecification().framebuffer;

		m_camera = CreateRef<PerspectiveCameraController>(60.f, 0.1f, 100.f);
		m_camera->SetPosition({ 0.f, 0.f, 3.f });

		m_saveIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/saveIcon.png");
		m_searchIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/searchIcon.png");
		m_reloadIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/reloadIcon.png");
		m_directoryIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/directoryIcon.png");

		m_materialModel = ResourceCache::GetAsset<Mesh>("assets/meshes/base/sphere.lgf");
		m_selectedMaterial = MaterialLibrary::GetMaterial("base");
		m_materialInstance = MaterialInstance::Create(m_selectedMaterial);

		m_directories[s_assetsPath.string()] = ProcessDirectory(s_assetsPath, nullptr);
	}

	void MaterialEditorPanel::OnEvent(Lamp::Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(MaterialEditorPanel::OnUpdate));
		dispatcher.Dispatch<ImGuiUpdateEvent>(LP_BIND_EVENT_FN(MaterialEditorPanel::OnUpdateImGui));
		dispatcher.Dispatch<KeyPressedEvent>(LP_BIND_EVENT_FN(MaterialEditorPanel::OnKeyPressed));
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(MaterialEditorPanel::OnRender));
	}

	bool MaterialEditorPanel::OnRender(Lamp::AppRenderEvent& e)
	{
		if (!m_isOpen)
		{
			return false;
		}

		RenderCommand::Begin(m_camera->GetCamera());
		RenderCommand::BeginPass(m_renderPipeline);
		RenderCommand::DrawMeshDirectWithPipeline(glm::mat4(1.f), m_materialModel->GetSubMeshes()[0], m_materialInstance, m_renderPipeline);
		RenderCommand::EndPass();
		RenderCommand::End();

		return false;
	}

	bool MaterialEditorPanel::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		return false;
	}

	void MaterialEditorPanel::UpdateToolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.305f, 0.31f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.505f, 0.51f, 0.5f));

		ImGui::Begin("##toolbarMaterialEditor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 4.f;

		if (ImGui::ImageButton(UI::GetTextureID(m_saveIcon), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			g_pEnv->pAssetManager->SaveAsset(m_selectedMaterial);
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	bool MaterialEditorPanel::OnUpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		ScopedTimer timer{};

		if (!m_isOpen)
		{
			return false;
		}

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		
		ImGui::Begin(m_name.c_str(), &m_isOpen);

		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID(m_name.c_str());
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		ImGui::End();

		UpdateProperties();
		UpdateMaterialView();
		UpdateMaterialList();
		UpdateToolbar();

		m_deltaTime = timer.GetTime();
		return false;
	}

	bool MaterialEditorPanel::OnKeyPressed(Lamp::KeyPressedEvent& e)
	{
		switch (e.GetKeyCode())
		{
			case LP_KEY_S:
			{
				bool control = Input::IsKeyPressed(LP_KEY_LEFT_CONTROL) || Input::IsKeyPressed(LP_KEY_RIGHT_CONTROL);
				if (control && m_selectedMaterial)
				{
					g_pEnv->pAssetManager->SaveAsset(m_selectedMaterial);
					LP_CORE_INFO("Saved material {0}", m_selectedMaterial->GetName());
				}
			}
		}

		return false;
	}

	Ref<DirectoryData> MaterialEditorPanel::ProcessDirectory(const std::filesystem::path& path, Ref<DirectoryData> parent)
	{
		Ref<DirectoryData> dirData = CreateRef<DirectoryData>();
		dirData->path = path;
		dirData->parent = parent.get();

		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (!entry.is_directory())
			{
				AssetType type = g_pEnv->pAssetManager->GetAssetTypeFromPath(entry);
				if (type == AssetType::Material)
				{
					AssetData data;
					data.path = entry;
					data.handle = g_pEnv->pAssetManager->GetAssetHandleFromPath(entry);
					data.type = type;

					dirData->assets.emplace_back(data);
				}
			}
			else
			{
				dirData->subDirectories.emplace_back(ProcessDirectory(entry, dirData));
			}
		}

		for (int32_t i = (int32_t)dirData->subDirectories.size() - 1; i >= 0; --i)
		{
			if (dirData->subDirectories[i]->assets.empty() && dirData->subDirectories[i]->subDirectories.empty())
			{
				dirData->subDirectories.erase(dirData->subDirectories.begin() + i);
			}
		}

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

	void MaterialEditorPanel::Reload()
	{
	}

	void MaterialEditorPanel::Search(const std::string& query)
	{
	}

	void MaterialEditorPanel::FindAssetsWithQuery(const std::vector<Ref<DirectoryData>>& dirList, const std::string& query)
	{
	}

	void MaterialEditorPanel::RenderDirectory(Ref<DirectoryData> dirData)
	{
		std::string id = dirData->path.stem().string() + "##" + std::to_string(dirData->handle);
		auto flags = (dirData->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None);

		if (UI::TreeNodeImage(m_directoryIcon, id.c_str(), flags))
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

			UI::TreeNodePop();
		}

		glm::rotate(glm::mat4(1.f), 0.f, { 0.f, 1.f, 0.f });
	}

	void MaterialEditorPanel::UpdateMaterialView()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		ImGui::Begin("Material View");
		{
			ImVec2 panelSize = ImGui::GetContentRegionAvail();
			if (m_perspectiveSize != *((glm::vec2*)&panelSize))
			{
				m_perspectiveSize = { panelSize.x, panelSize.y };
				m_framebuffer->Resize((uint32_t)panelSize.x, (uint32_t)panelSize.y);

				m_camera->UpdateProjection((uint32_t)panelSize.x, (uint32_t)panelSize.y);
			}

			ImGui::Image(UI::GetTextureID(m_framebuffer->GetColorAttachment(0)), ImVec2{ m_perspectiveSize.x, m_perspectiveSize.y }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* wPath = (const wchar_t*)pPayload->Data;
					std::filesystem::path path(wPath);

					AssetType type = g_pEnv->pAssetManager->GetAssetTypeFromPath(path);
					if (type == Lamp::AssetType::Material)
					{
						m_selectedMaterial = ResourceCache::GetAsset<Material>(path);
						m_materialInstance = MaterialInstance::Create(m_selectedMaterial);
					}
				}

				ImGui::EndDragDropTarget();
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void MaterialEditorPanel::UpdateProperties()
	{
		ImGui::Begin("Properties##Material");

		if (!m_materialModel)
		{
			return;
		}

		if (!m_selectedMaterial)
		{
			m_selectedMaterial = m_materialModel->GetMaterials()[0];
		}

		ImGui::BeginChild("matProperties", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
		{
			if (m_selectedMaterial)
			{
				auto& materialData = const_cast<MaterialData&>(m_selectedMaterial->GetMaterialData());

				if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
				{
					if (UI::TreeNode("Colors", ImGuiTreeNodeFlags_DefaultOpen))
					{
						UI::PushId();
						if (UI::BeginProperties("colorProperties", false))
						{
							UI::Property("Use albedo", materialData.useAlbedo);
							if (!materialData.useAlbedo)
							{
								UI::PropertyColor("Albedo color", materialData.albedoColor);
							}

							UI::Property("Use normal", materialData.useNormal);
							if (!materialData.useNormal)
							{
								UI::PropertyColor("Normal color", materialData.normalColor);
							}

							UI::Property("Use MRO", materialData.useMRO);
							if (!materialData.useMRO)
							{
								UI::Property("MRO", materialData.mroColor);
							}

							UI::EndProperties(false);
						}
						UI::PopId();
						UI::TreeNodePop();
					}

					if (UI::TreeNode("Blending", ImGuiTreeNodeFlags_DefaultOpen))
					{
						UI::PushId();
						if (UI::BeginProperties("blendingProperties", false))
						{
							UI::Property("Use blending", materialData.useBlending);
							UI::Property("Blending multiplier", materialData.blendingMultiplier);
							UI::EndProperties(false);
						}
						UI::PopId();
						UI::TreeNodePop();
					}

					if (UI::TreeNode("Other"))
					{
						UI::PushId();
						if (UI::BeginProperties("otherProperties", false))
						{
							UI::Property("Use translucency", materialData.useTranslucency);
							UI::Property("Use detail normals", materialData.useDetailNormal);

							UI::EndProperties(false);
						}
						UI::PopId();
						UI::TreeNodePop();
					}
				}

				if (ImGui::CollapsingHeader("Textures"))
				{
					for (auto& textureSpec : const_cast<std::vector<Material::MaterialTextureSpecification>&>(m_selectedMaterial->GetTextureSpecification()))
					{
						std::string buttonId = "##" + textureSpec.name;
						Ref<Texture2D> texture = textureSpec.texture;
						if (!texture)
						{
							texture = Renderer::Get().GetDefaults().whiteTexture;
						}

						ImGui::TextUnformatted(textureSpec.name.c_str());
						bool pressed = UI::ImageButton(buttonId.c_str(), UI::GetTextureID(textureSpec.texture), { 128.f, 128.f });
						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
							{
								const wchar_t* wPath = (const wchar_t*)pPayload->Data;
								std::filesystem::path path(wPath);

								AssetType type = g_pEnv->pAssetManager->GetAssetTypeFromPath(path);
								if (type == Lamp::AssetType::Texture)
								{
									textureSpec.texture = ResourceCache::GetAsset<Texture2D>(path);
								}
							}

							ImGui::EndDragDropTarget();
						}

						if (pressed)
						{
							std::filesystem::path newPath = FileDialogs::OpenFile("All (*.*)\0*.*)");
							if (std::filesystem::exists(newPath))
							{
								textureSpec.texture = ResourceCache::GetAsset<Texture2D>(newPath);
							}
						}

					}
				}
			}
		}
		ImGui::EndChild();

		ImGui::End();
	}

	void MaterialEditorPanel::UpdateMaterialList()
	{
		ImGui::Begin("Materials##matEd");

		const float buttonSize = 20.f;

		if (UI::ImageButton("##reloadButton", UI::GetTextureID(m_reloadIcon), { buttonSize, buttonSize }))
		{
			MaterialLibrary::Get().LoadMaterials();
		}

		ImGui::SameLine();
		UI::ShiftCursor(0.f, 2.f);
		ImGui::Image(UI::GetTextureID(m_searchIcon), { buttonSize, buttonSize }, { 1.f, 1.f }, { 0.f, 0.f });
		ImGui::SameLine();

		UI::ShiftCursor(0.f, -2.f);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

		if (UI::InputText("##searchBar", m_searchQuery, ImGuiInputTextFlags_EnterReturnsTrue))
		{

		}
		ImGui::PopItemWidth();

		UI::ScopedColor childColor(ImGuiCol_ChildBg, { 0.18f, 0.18f, 0.18f, 1.f });
		UI::PushId();

		if (ImGui::BeginChild("materials", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{
			if (UI::TreeNodeImage(m_directoryIcon, "Assets", ImGuiTreeNodeFlags_DefaultOpen))
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


		UI::PopId();

		ImGui::End();
	}

	void MaterialEditorPanel::CreateNewMaterial()
	{
		//std::filesystem::path matPath = s_assetsPath / "material.mtl";

		//Ref<Material> mat = Material::Create("New Material", 0);
		//mat->Path = matPath;
		//mat->SetShader(ShaderLibrary::GetShader("pbrForward"));

		//g_pEnv->pAssetManager->SaveAsset(mat);
		//MaterialLibrary::Get().AddMaterial(mat);
	}
}