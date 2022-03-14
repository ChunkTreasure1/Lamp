#include "MeshImporterPanel.h"

#include <imgui/imgui.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Mesh/Materials/MaterialLibrary.h>
#include <Lamp/Mesh/Materials/MaterialInstance.h>

#include <Lamp/Rendering/RenderCommand.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Rendering/RenderPipelineLibrary.h>

#include <Lamp/AssetSystem/AssetManager.h>
#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/AssetSystem/BaseAssets.h>

#include <Lamp/Utility/UIUtility.h>
#include <Lamp/Core/Time/ScopedTimer.h>

namespace Sandbox
{
	using namespace Lamp;

	MeshImporterPanel::MeshImporterPanel(std::string_view name)
		: EditorWindow(name)
	{
		m_renderPipeline = RenderPipelineLibrary::Get().GetPipeline(ERenderPipeline::Forward);
		m_framebuffer = m_renderPipeline->GetSpecification().framebuffer;

		m_camera = CreateRef<PerspectiveCameraController>(60.f, 0.01f, 100.f);
		m_camera->SetPosition({ 0.f, 0.f, 3.f });

		//Setup icons
		m_loadIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/loadIcon.png");
		m_saveIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/saveIcon.png");
	}

	bool MeshImporterPanel::OnUpdateImGui(Lamp::ImGuiUpdateEvent& e)
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
			std::string id = m_name + "dockspace";
			ImGuiID dockspace_id = ImGui::GetID(id.c_str());
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		ImGui::End();

		UpdatePerspective();
		UpdateProperties();
		UpdateMaterial();
		UpdateToolbar();
		UpdateMeshConstruction();

		m_deltaTime = timer.GetTime();

		return false;
	}

	bool MeshImporterPanel::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		if (m_defaultShader == nullptr)
		{
			m_defaultShader = Lamp::ShaderLibrary::GetShader("pbrForward");
		}

		if (m_meshToImport)
		{
			for (const auto& mesh : m_meshToImport->GetSubMeshes())
			{
				RenderCommand::SubmitMesh(m_transform, mesh, m_materialInstances[mesh->GetMaterialIndex()]);
			}
		}

		UpdateCamera(e.GetTimestep());

		if (Input::IsMouseButtonReleased(1))
		{
			m_camera->SetHasControl(false);
			m_camera->SetActive(false);
			m_rightMousePressed = false;

			Application::Get().GetWindow().ShowCursor(true);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}

		if (Input::IsMouseButtonPressed(1) && (m_hoveringPerspective || m_rightMousePressed))
		{
			m_camera->OnEvent(e);
			m_camera->SetActive(true);
			m_camera->Update(e.GetTimestep());
			m_rightMousePressed = true;

			Application::Get().GetWindow().ShowCursor(false);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}

		return false;
	}

	std::string MeshImporterPanel::GetDragDropTarget()
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)pPayload->Data;
				std::filesystem::path p = std::filesystem::path("assets") / path;
				return p.string();
			}

			ImGui::EndDragDropTarget();
		}

		return "";
	}

	void MeshImporterPanel::MaterialPopup()
	{
	}

	void MeshImporterPanel::UpdateCamera(Lamp::Timestep ts)
	{
		if (!m_isOpen)
		{
			return;
		}

		if (Input::IsMouseButtonPressed(1) && (m_hoveringPerspective || m_rightMousePressed))
		{
			m_camera->Update(ts);
		}
	}

	void MeshImporterPanel::UpdateToolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.305f, 0.31f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.505f, 0.51f, 0.5f));

		ImGui::Begin("##toolbarMeshImporter", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 4.f;

		if (UI::ImageButton("##load", UI::GetTextureID(m_loadIcon), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			m_importSettings.path = FileDialogs::OpenFile("FBX File (*.fbx)\0*.fbx\0");
			if (!m_importSettings.path.empty() && std::filesystem::exists(m_importSettings.path))
			{
				LoadMesh();
			}
		}

		ImGui::SameLine();

		if (UI::ImageButton("##save", UI::GetTextureID(m_saveIcon), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			m_savePath = Lamp::FileDialogs::SaveFile("Lamp Geometry (*.lgf)\0*.lgf\0");
			SaveMesh();
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void MeshImporterPanel::UpdateMeshConstruction()
	{
	}

	bool MeshImporterPanel::OnRender(Lamp::AppRenderEvent& e)
	{
		if (!m_isOpen)
		{
			return false;
		}

		RenderCommand::Begin(m_camera->GetCamera());
		RenderCommand::BeginPass(m_renderPipeline);
		RenderCommand::DispatchRenderCommands();
		RenderCommand::EndPass();
		RenderCommand::End();

		return false;
	}

	void MeshImporterPanel::LoadMesh()
	{
		m_meshToImport = MeshImporter::ImportMesh(m_importSettings);
		for (auto& material : m_meshToImport->GetMaterials())
		{
			material.second->SetRenderPipeline(RenderPipelineLibrary::Get().GetPipeline(Lamp::ERenderPipeline::Forward));
		}

		m_transform = glm::mat4(1.f);
		m_scale = glm::vec3(1.f);

		m_savePath = m_importSettings.path.stem().string();
		m_savePath += ".lgf";
		m_meshToImport->Path = m_savePath;

		m_materialInstances.clear();
		for (const auto& mat : m_meshToImport->GetMaterials())
		{
			m_materialInstances.emplace_back(MaterialInstance::Create(mat.second));
		}

		m_shaderSelectionIds.clear();
		for (int i = 0; i < m_meshToImport->GetMaterials().size(); i++)
		{
			m_shaderSelectionIds.push_back(0);
		}
	}

	void MeshImporterPanel::SaveMesh()
	{
		if (!m_savePath.empty())
		{
			m_savePath = std::filesystem::path("assets") / std::filesystem::relative(m_savePath, "assets");

			if (m_savePath.extension().empty())
			{
				std::string path = m_savePath.string();
				path += ".lgf";
				m_savePath = std::filesystem::path(path);
			}

			m_meshToImport->Path = m_savePath;

			for (auto& mat : m_meshToImport->GetMaterials())
			{
				if (!MaterialLibrary::Get().IsMaterialLoaded(mat.second->GetName()))
				{
					std::string sMatPath = m_savePath.parent_path().string() + "/" + mat.second->GetName() + ".mtl";

					mat.second->Path = std::filesystem::path(sMatPath);
					g_pEnv->pAssetManager->SaveAsset(mat.second);

					MaterialLibrary::Get().AddMaterial(mat.second);
				}
			}

			//Save mesh file
			g_pEnv->pAssetManager->SaveAsset(m_meshToImport);
			m_savePath = "";

			//Copy and create MeshSource
			if (std::filesystem::exists(m_importSettings.path))
			{
				std::filesystem::path dest = m_importSettings.path.filename();

				dest = m_meshToImport->Path.parent_path() / dest;

				std::filesystem::copy_file(m_importSettings.path, dest, std::filesystem::copy_options::overwrite_existing);

				m_importSettings.path = dest;
				Ref<MeshSource> meshSource = CreateRef<MeshSource>(m_importSettings, m_meshToImport->Handle);
				meshSource->Path = m_meshToImport->Path.parent_path() / std::filesystem::path(m_importSettings.path.stem().string() + ".lgs");

				g_pEnv->pAssetManager->SaveAsset(meshSource);
			}
		}
	}

	void MeshImporterPanel::OnEvent(Lamp::Event& e)
	{
		if (!m_isOpen)
		{
			return;
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::ImGuiUpdateEvent>(LP_BIND_EVENT_FN(MeshImporterPanel::OnUpdateImGui));
		dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(MeshImporterPanel::OnUpdate));
		dispatcher.Dispatch<Lamp::AppRenderEvent>(LP_BIND_EVENT_FN(MeshImporterPanel::OnRender));
	}

	void MeshImporterPanel::UpdatePerspective()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Import Perspective");
		{
			glm::vec2 windowPos = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
			m_hoveringPerspective = ImGui::IsWindowHovered();
			m_camera->SetControlsEnabled(m_hoveringPerspective);

			ImVec2 panelSize = ImGui::GetContentRegionAvail();
			if (m_perspectiveSize != *((glm::vec2*)&panelSize))
			{
				m_framebuffer->Resize((uint32_t)panelSize.x, (uint32_t)panelSize.y);
				m_perspectiveSize = { panelSize.x, panelSize.y };

				m_camera->UpdateProjection((uint32_t)panelSize.x, (uint32_t)panelSize.y);
			}

			ImGui::Image(UI::GetTextureID(m_framebuffer->GetColorAttachment(0)), ImVec2{ m_perspectiveSize.x, m_perspectiveSize.y }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void MeshImporterPanel::UpdateProperties()
	{
		ImGui::Begin("Import Settings", &m_isOpen);

		UI::ScopedColor childColor(ImGuiCol_ChildBg, { 0.18f, 0.18f, 0.18f, 1.f });


		if (ImGui::BeginChild("properties", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 5.f, 5.f });

			ImGui::Text(("Source path: " + m_importSettings.path.string()).c_str());
			ImGui::Text(("Destination path: " + m_savePath.string()).c_str());

			UI::PushId();
			if (UI::BeginProperties("test", false))
			{
				if (UI::Property("Show Skybox", m_renderSkybox))
				{
				}

				UI::Property("Show Grid", m_renderGrid);
				if (UI::Property("Compile static", m_importSettings.compileStatic))
				{
					LoadMesh();
				}

				UI::EndProperties(false);
			}
			UI::PopId();

			ImGui::Separator();
			ImGui::Text("Mesh settings");

			static std::vector<const char*> units = { "Centimeters", "Decimeters", "Meters" };
			static int currentUnit = 0;

			static std::vector<const char*> meshDirections = { "Y+ up", "Y- up", "Z+ up", "Z- up", "X+ up", "X- up" };
			static int currentDirection = 0;

			UI::PushId();
			if (UI::BeginProperties("meshSettings", false))
			{
				if (UI::PropertyAxisColor("Scale", m_scale, 0.f))
				{
					m_transform = glm::scale(glm::mat4(1.f), m_scale);
				}

				if (UI::Combo("Units", currentUnit, units))
				{
					m_importSettings.units = static_cast<Units>(currentUnit);
					LoadMesh();
				}

				if (UI::Combo("Up direction", currentDirection, meshDirections))
				{
				}

				UI::EndProperties(false);
			}
			UI::PopId();

			ImGui::PopStyleVar();
			ImGui::EndChild();
		}

		ImGui::End();
	}

	void MeshImporterPanel::UpdateMaterial()
	{
		ImGui::Begin("Materials");

		static std::vector<const char*> shaders;

		shaders.clear();
		for (auto& shader : ShaderLibrary::GetShaders())
		{
			shaders.push_back(shader->GetName().c_str());
		}

		if (m_meshToImport.get())
		{
			int matId = 0;
			for (auto& mat : m_meshToImport->GetMaterials())
			{
				for (int i = 0; i < shaders.size(); i++)
				{
					if (mat.second->GetShader()->GetName() == shaders[i])
					{
						m_shaderSelectionIds[matId] = i;
					}
				}
				matId++;
			}

			int i = 0;
			for (auto& mat : m_meshToImport->GetMaterials())
			{
				std::string id = mat.second->GetName() + "###mat" + std::to_string(i);
				if (ImGui::CollapsingHeader(id.c_str()))
				{
					std::string propId = "##props" + std::to_string(i);
					UI::PushId();
					if (UI::BeginProperties(propId, false))
					{
						UI::Property("Name", mat.second->GetName());

						if (UI::Combo("Shader", m_shaderSelectionIds[i], shaders))
						{
							if (mat.second->GetShader() != ShaderLibrary::GetShader(shaders[m_shaderSelectionIds[i]]))
							{
								//mat.second->SetShader(ShaderLibrary::GetShader(shaders[m_shaderSelectionIds[i]]));
							}
						}

						UI::EndProperties(false);
					}
					UI::PopId();

					ImGui::Separator();

					//TODO: fix
					//for (auto& tex : const_cast<std::unordered_map<std::string, Ref<Texture2D>>&>(mat.second->GetTextures()))
					//{
					//	ImGui::Text(tex.first.c_str());

					//	std::filesystem::path path;
					//	if (UI::ImageButton(tex.second->GetID(), path))
					//	{
					//		if (!path.empty())
					//		{
					//			Ref<Texture2D> newTex = ResourceCache::GetAsset<Texture2D>(path);
					//			if (newTex->IsValid())
					//			{
					//				tex.second = newTex;
					//			}
					//		}
					//	}

					//	if (auto ptr = UI::DragDropTarget("CONTENT_BROWSER_ITEM"))
					//	{
					//		const wchar_t* wPath = (const wchar_t*)ptr;
					//		std::filesystem::path path(wPath);
					//		AssetType type = g_pEnv->pAssetManager->GetAssetTypeFromPath(path);
					//		if (type == AssetType::Texture)
					//		{
					//			tex.second = ResourceCache::GetAsset<Texture2D>(path);
					//		}
					//	}
					//	ImGui::Separator();
					//}
				}

				i++;
			}
		}

		ImGui::End();
	}
}