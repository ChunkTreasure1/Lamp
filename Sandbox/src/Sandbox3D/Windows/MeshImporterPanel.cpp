#include "MeshImporterPanel.h"

#include <Lamp/Rendering/RenderCommand.h>
#include <imgui/imgui.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Mesh/Materials/MaterialLibrary.h>
#include <imgui/imgui_stdlib.h>

#include <Platform/OpenGL/OpenGLFramebuffer.h>
#include <Lamp/AssetSystem/MeshImporter.h>
#include <Lamp/AssetSystem/AssetManager.h>
#include <Lamp/AssetSystem/ResourceCache.h>

#include <Lamp/Utility/UIUtility.h>

namespace Sandbox3D
{
	using namespace Lamp;

	MeshImporterPanel::MeshImporterPanel(std::string_view name)
		: BaseWindow(name)
	{
		m_Camera = CreateRef<PerspectiveCameraController>(60.f, 0.01f, 100.f);
		m_Camera->SetPosition({ -3.f, 2.f, 3.f });

		m_RenderFuncs.push_back(LP_EXTRA_RENDER(MeshImporterPanel::Render));

		//Setup icons
		m_LoadIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/loadIcon.png");
		m_SaveIcon = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/saveIcon.png");

		{
			FramebufferSpecification mainBuffer;
			mainBuffer.Attachments =
			{
				{ FramebufferTextureFormat::RGBA8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge },
				{ FramebufferTextureFormat::DEPTH24STENCIL8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge }
			};
			mainBuffer.ClearColor = { 0.1f, 0.15f, 0.15f, 1.f };
			mainBuffer.Height = 720;
			mainBuffer.Width = 1280;

			RenderPassSpecification passSpec{};
			passSpec.TargetFramebuffer = Lamp::Framebuffer::Create(mainBuffer);
			passSpec.clearType = Lamp::ClearType::ColorDepth;
			passSpec.cullFace = Lamp::CullFace::Back;

			m_RenderPass = CreateRef<RenderPass>(passSpec);
		}
	}

	bool MeshImporterPanel::UpdateImGui(Lamp::ImGuiUpdateEvent& e)
	{
		if (!m_IsOpen)
		{
			return false;
		}

		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin(m_Name.c_str(), &m_IsOpen);
		ImGui::PopStyleVar();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID(m_Name.c_str());
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		ImGui::End();

		UpdatePerspective();
		UpdateProperties();
		UpdateMaterial();
		UpdateToolbar();
		UpdateMeshConstruction();

		return false;
	}

	bool MeshImporterPanel::Update(Lamp::AppUpdateEvent& e)
	{
		if (m_DefaultShader == nullptr)
		{
			m_DefaultShader = Lamp::ShaderLibrary::GetShader("pbrForward");
		}
		UpdateCamera(e.GetTimestep());

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
		if (!m_IsOpen)
		{
			return;
		}

		if (Input::IsMouseButtonPressed(1) && (m_HoveringPerspective || m_RightMousePressed))
		{
			m_Camera->Update(ts);
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

		if (ImGui::ImageButton((ImTextureID)m_LoadIcon->GetID(), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			m_SourcePath = FileDialogs::OpenFile("FBX File (*.fbx)\0*.fbx\0");
			if (m_SourcePath != "" && std::filesystem::exists(m_SourcePath))
			{
				m_pModelToImport = MeshImporter::ImportMesh(m_SourcePath);

				m_SavePath = m_SourcePath.substr(0, m_SourcePath.find_last_of('.'));
				m_SavePath += ".lgf";
				m_pModelToImport->Path = m_SavePath;

				for (auto& mat : m_pModelToImport->GetMaterials())
				{
					mat.second->SetShader(m_DefaultShader);

					for (auto& tex : mat.second->GetTextures())
					{
						tex.second = ResourceCache::GetAsset<Texture2D>("engine/textures/default/defaultTexture.png");
					}
				}

				m_ShaderSelectionIds.clear();
				for (int i = 0; i < m_pModelToImport->GetMaterials().size(); i++)
				{
					m_ShaderSelectionIds.push_back(0);
				}
			}
		}

		ImGui::SameLine();

		if (ImGui::ImageButton((ImTextureID)m_SaveIcon->GetID(), { size, size }, { 0.f, 1.f }, { 1.f, 0.f }, 0))
		{
			m_SavePath = Lamp::FileDialogs::SaveFile("Lamp Geometry (*.lgf)\0*.lgf\0");

			std::string matOriginPath = m_SavePath.substr(m_SavePath.find_last_of('\\') + 1, m_SavePath.size() - 1);
			

			if (m_SavePath != "")
			{
				if (m_SavePath.find(".lgf") == std::string::npos)
				{
					m_SavePath += ".lgf";
				}

				m_pModelToImport->Path = m_SavePath;

				for (auto& mat : m_pModelToImport->GetMaterials())
				{
					mat.second->SetName(matOriginPath + std::to_string(mat.first));
					if (!MaterialLibrary::IsMaterialLoaded(matOriginPath + std::to_string(mat.first)))
					{
						std::string matPath = m_SavePath.substr(0, m_SavePath.find_last_of('\\') + 1);
						matPath += matOriginPath + ".mtl";
						//Add material to library
						
						//TODO: fix
						//MaterialLibrary::SaveMaterial(matPath, m_pModelToImport->GetMaterial(mat.first));
						//MaterialLibrary::AddMaterial(m_pModelToImport->GetMaterial(mat.first));
					}
				}

				g_pEnv->pAssetManager->SaveAsset(m_pModelToImport);
				m_SavePath = "";
			}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void MeshImporterPanel::UpdateMeshConstruction()
	{
	}

	void MeshImporterPanel::Render()
	{
		if (!m_IsOpen)
		{
			return;
		}

		Renderer3D::Begin(m_Camera->GetCamera());


		Renderer3D::End();
	}

	void MeshImporterPanel::OnEvent(Lamp::Event& e)
	{
		if (!m_IsOpen)
		{
			return;
		}

		if (Input::IsMouseButtonReleased(1))
		{
			m_Camera->SetHasControl(false);
			m_RightMousePressed = false;

			Application::Get().GetWindow().ShowCursor(true);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}

		if (Input::IsMouseButtonPressed(1) && (m_HoveringPerspective || m_RightMousePressed))
		{
			m_Camera->OnEvent(e);
			m_RightMousePressed = true;

			Application::Get().GetWindow().ShowCursor(false);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::ImGuiUpdateEvent>(LP_BIND_EVENT_FN(MeshImporterPanel::UpdateImGui));
		dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(MeshImporterPanel::Update));
	}

	void MeshImporterPanel::UpdatePerspective()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Import Perspective");
		{
			glm::vec2 windowPos = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
			m_HoveringPerspective = ImGui::IsWindowHovered();
			m_Camera->SetControlsEnabled(m_HoveringPerspective);

			ImVec2 panelSize = ImGui::GetContentRegionAvail();
			if (m_PerspectiveSize != *((glm::vec2*)&panelSize))
			{
				m_RenderPass->GetSpecification().TargetFramebuffer->Resize((uint32_t)panelSize.x, (uint32_t)panelSize.y);
				m_PerspectiveSize = { panelSize.x, panelSize.y };

				m_Camera->UpdateProjection((uint32_t)panelSize.x, (uint32_t)panelSize.y);
			}

			uint32_t textureID = m_RenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentID();
			ImGui::Image((void*)(uint64_t)textureID, ImVec2{ panelSize.x, panelSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void MeshImporterPanel::UpdateProperties()
	{
		ImGui::Begin("Import Settings", &m_IsOpen);

		ImGui::Text(("Source path: " + m_SourcePath).c_str());
		ImGui::Text(("Destination path: " + m_SavePath).c_str());

		ImGui::Checkbox("Show Skybox", &m_RenderSkybox);
		ImGui::Checkbox("Show Grid", &m_RenderGrid);

		ImGui::Separator();
		ImGui::Text("Mesh settings");

		if (ImGui::DragFloat("Scale", &m_ImportSettings.MeshScale) && m_pModelToImport)
		{
		}

		static const char* meshDirections[] = { "Y+ up", "Y- up", "Z+ up", "Z- up", "X+ up", "X- up" };
		static int currentDirection = 0;
		ImGui::Combo("Up", &currentDirection, meshDirections, IM_ARRAYSIZE(meshDirections));

		ImGui::End();
	}

	void MeshImporterPanel::UpdateMaterial()
	{
		ImGui::Begin("Materials");

		static std::vector<const char*> shaders;
		static std::unordered_map<std::string, std::string> paths;

		UI::ImageText(m_SaveIcon->GetID(), "test");

		shaders.clear();
		for (auto& shader : ShaderLibrary::GetShaders())
		{
			shaders.push_back(shader->GetName().c_str());
		}

		if (m_pModelToImport.get())
		{
			int matId = 0;
			for (auto& mat : m_pModelToImport->GetMaterials())
			{
				for (int i = 0; i < shaders.size(); i++)
				{
					if (mat.second->GetShader()->GetName() == shaders[i])
					{
						m_ShaderSelectionIds[matId] = i;
					}
				}
				matId++;
			}

			int i = 0;
			for (auto& mat : m_pModelToImport->GetMaterials())
			{
				std::string id = mat.second->GetName() + "###mat" + std::to_string(i);
				if (ImGui::CollapsingHeader(id.c_str()))
				{
					std::string matName = mat.second->GetName();
					std::string nameId = "Name##matName" + std::to_string(i);
					if (ImGui::InputText(nameId.c_str(), &matName))
					{
						mat.second->SetName(matName);
					}

					std::string comboId = "Shader##shader" + std::to_string(i);
					if (ImGui::Combo(comboId.c_str(), &m_ShaderSelectionIds[i], shaders.data(), shaders.size()))
					{
						if (mat.second->GetShader() != ShaderLibrary::GetShader(shaders[m_ShaderSelectionIds[i]]))
						{
							mat.second->SetShader(ShaderLibrary::GetShader(shaders[m_ShaderSelectionIds[i]]));

						}
					}

					ImGui::Separator();

					for (auto& tex : mat.second->GetTextures())
					{
						ImGui::Text(tex.first.c_str());
						if (ImGui::ImageButton((ImTextureID)tex.second->GetID(), { 64, 64 }, { 0, 1 }, { 1, 0 }))
						{

						}
						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
							{
								const wchar_t* wPath = (const wchar_t*)pPayload->Data;
								std::filesystem::path path(wPath);
								AssetType type = g_pEnv->pAssetManager->GetAssetTypeFromPath(path);
								if (type == AssetType::Texture)
								{
									tex.second = ResourceCache::GetAsset<Texture2D>(std::filesystem::path("assets") / path);
								}
							}

							ImGui::EndDragDropTarget();
						}
						ImGui::Separator();
					}
				}

				i++;
			}
		}

		ImGui::End();
	}
}