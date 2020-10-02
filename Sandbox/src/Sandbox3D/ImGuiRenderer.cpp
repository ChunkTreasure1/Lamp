#include "lppch.h"
#include "Sandbox3D.h"

#include <Lamp/Level/LevelSystem.h>
#include <Lamp/Meshes/GeometrySystem.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <Lamp/Objects/ObjectLayer.h>

#include <Lamp/Objects/Entity/Base/Entity.h>
#include <Lamp/Objects/Entity/Base/ComponentRegistry.h>
#include <ImGuizmo/ImGuizmo.h>

#include <Lamp/Core/CoreLogger.h>

namespace Sandbox3D
{
	void Sandbox3D::UpdatePerspective()
	{
		if (!m_PerspectiveOpen)
		{
			return;
		}

		glm::vec2 perspectivePos;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Perspective");
		{
			perspectivePos = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
			m_PerspectiveHover = ImGui::IsWindowHovered();
			m_SandboxController->GetCameraController()->SetControlsEnabled(m_PerspectiveHover);

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Aspect ratio"))
				{

				}
				ImGui::EndMenuBar();
			}

			ImVec2 perspectivePanelSize = ImGui::GetContentRegionAvail();
			if (m_PerspectiveSize != *((glm::vec2*) & perspectivePanelSize))
			{
				Lamp::Renderer3D::GetFrameBuffer()->Update((uint32_t)perspectivePanelSize.x, (uint32_t)perspectivePanelSize.y);
				m_PerspectiveSize = { perspectivePanelSize.x, perspectivePanelSize.y };

				m_SandboxController->GetCameraController()->UpdateProjection(perspectivePanelSize.x, perspectivePanelSize.y);
			}

			uint32_t textureID = Lamp::Renderer3D::GetFrameBuffer()->GetColorAttachment();
			ImGui::Image((void*)(uint64_t)textureID, ImVec2{ m_PerspectiveSize.x, m_PerspectiveSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

			std::string frameInfo = "FrameTime: " + std::to_string(Lamp::Application::Get().GetFrameTime().GetFrameTime()) + ". FPS: " + std::to_string(Lamp::Application::Get().GetFrameTime().GetFramesPerSecond()) + ". Using VSync: " + std::to_string(Lamp::Application::Get().GetWindow().GetIsVSync());
			ImGui::SetCursorPos(ImVec2(20, 40));
			ImGui::Text(frameInfo.c_str());
		}
		ImGui::End();
		ImGui::PopStyleVar();

		static glm::mat4 transform = glm::mat4(1.f);

		if (m_pSelectedObject)
		{
			transform = m_pSelectedObject->GetModelMatrix();
			ImGuizmo::SetRect(perspectivePos.x, perspectivePos.y, m_PerspectiveSize.x, m_PerspectiveSize.y);
			ImGuizmo::Manipulate(glm::value_ptr(m_SandboxController->GetCameraController()->GetCamera()->GetViewMatrix()),
				glm::value_ptr(m_SandboxController->GetCameraController()->GetCamera()->GetProjectionMatrix()),
				m_ImGuizmoOperation, ImGuizmo::WORLD, glm::value_ptr(transform));

			m_pSelectedObject->SetModelMatrix(transform);
		}
	}

	void Sandbox3D::UpdateAssetBrowser()
	{
		if (!m_AssetBrowserOpen)
		{
			return;
		}

		ImGui::Begin("Asset Browser", &m_AssetBrowserOpen);
		{
			//Asset browser
			{
				ImGui::BeginChild("Browser", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.12f, ImGui::GetWindowSize().y * 0.85f), true);
				{
					std::vector<std::string> folders = Lamp::FileSystem::GetAssetFolders();

					Lamp::FileSystem::PrintFoldersAndFiles(folders);
				}
				ImGui::EndChild();

				ImGui::SameLine();
				ImGui::BeginChild("Viewer", ImVec2(ImGui::GetWindowSize().y * 0.85f, ImGui::GetWindowSize().y * 0.85f), true);
				{
					if (m_SelectedFile.GetFileType() == Lamp::FileType::Texture)
					{
						Ref<Lamp::Texture2D> selected;
						selected = Lamp::Texture2D::Create(m_SelectedFile.GetPath());
						ImGui::Image((void*)(uint64_t)selected->GetID(), ImVec2(ImGui::GetWindowSize().y * 0.9f, ImGui::GetWindowSize().y * 0.9f));
					}
				}
				ImGui::EndChild();
			}
		}
		ImGui::End();
	}

	void Sandbox3D::UpdateProperties()
	{
		if (!m_InspectiorOpen)
		{
			return;
		}
		ImGui::Begin("Properties", &m_InspectiorOpen);
		{

			ImGuiIO& io = ImGui::GetIO();
			glm::vec2 mousePos = glm::vec2(io.MouseClickedPos->x, io.MouseClickedPos->y);
			glm::vec2 windowPos;
			glm::vec2 windowSize;

			ImGui::Begin("Perspective");
			{
				windowPos = glm::vec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
				windowSize = glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
			}
			ImGui::End();

			if (m_MousePressed && m_PerspectiveHover && !ImGuizmo::IsOver())
			{
				mousePos -= windowPos;
				m_pSelectedObject = Lamp::ObjectLayerManager::Get()->GetObjectFromPoint(m_SandboxController->GetCameraController()->ScreenToWorldCoords(mousePos, m_PerspectiveSize), m_SandboxController->GetCameraController()->GetPosition());
			}

			if (auto pEnt = dynamic_cast<Lamp::Entity*>(m_pSelectedObject))
			{
				ImGui::Text("Entity");

				std::string name = pEnt->GetName();
				//ImGui::InputText("Name", &name);
				pEnt->SetName(name);

				if (ImGui::CollapsingHeader("Transform"))
				{
					glm::vec3 pos = pEnt->GetPosition();
					float f[3] = { pos.x, pos.y, pos.z };

					ImGui::InputFloat3("Position", f);
					pEnt->SetPosition(glm::make_vec3(f));

					glm::vec3 rot = pEnt->GetRotation();
					float r[3] = { rot.x, rot.y, rot.z };

					ImGui::InputFloat3("Rotation", r);
					pEnt->SetRotation(glm::make_vec3(r));

					glm::vec3 scale = pEnt->GetScale();
					float s[3] = { scale.x, scale.y, scale.z };

					ImGui::InputFloat3("Scale", s);
					pEnt->SetScale(glm::make_vec3(s));
				}

				for (auto& pComp : pEnt->GetComponents())
				{
					if (ImGui::CollapsingHeader(pComp->GetName().c_str()))
					{
						for (auto& pProp : pComp->GetComponentProperties().GetProperties())
						{
							switch (pProp.PropertyType)
							{
							case Lamp::PropertyType::Int:
							{
								int* p = static_cast<int*>(pProp.Value);
								ImGui::InputInt(pProp.Name.c_str(), p);
								break;
							}

							case Lamp::PropertyType::Bool:
							{
								bool* p = static_cast<bool*>(pProp.Value);
								ImGui::Checkbox(pProp.Name.c_str(), p);
								break;
							}

							case Lamp::PropertyType::Float:
							{
								float* p = static_cast<float*>(pProp.Value);
								ImGui::InputFloat(pProp.Name.c_str(), p);
								break;
							}

							case Lamp::PropertyType::Float2:
							{
								glm::vec2* p = static_cast<glm::vec2*>(pProp.Value);

								float f[2] = { p->x, p->y };
								ImGui::InputFloat2(pProp.Name.c_str(), f, 3);

								*p = glm::make_vec2(f);
								break;
							}

							case Lamp::PropertyType::Float3:
							{
								glm::vec3* p = static_cast<glm::vec3*>(pProp.Value);

								float f[3] = { p->x, p->y, p->z };
								ImGui::InputFloat3(pProp.Name.c_str(), f, 3);

								*p = glm::make_vec3(f);
								break;
							}

							case Lamp::PropertyType::Float4:
							{
								glm::vec4* p = static_cast<glm::vec4*>(pProp.Value);

								float f[4] = { p->x, p->y, p->z, p->w };
								ImGui::InputFloat4(pProp.Name.c_str(), f, 3);

								*p = glm::make_vec4(f);
								break;
							}

							case Lamp::PropertyType::String:
							{
								std::string* s = static_cast<std::string*>(pProp.Value);
								//ImGui::InputText(pProp.Name.c_str(), s);
								break;
							}

							case Lamp::PropertyType::Path:
							{
								std::string* s = static_cast<std::string*>(pProp.Value);
								//ImGui::InputText(pProp.Name.c_str(), s);
								ImGui::SameLine();
								if (ImGui::Button("Open"))
								{
									std::string path = Lamp::FileSystem::GetFileFromDialogue();
									if (path != "" && std::filesystem::exists(path))
									{
										*s = path;
									}
								}
								break;
							}

							case Lamp::PropertyType::Color3:
							{
								glm::vec3* p = static_cast<glm::vec3*>(pProp.Value);

								float f[3] = { p->x, p->y, p->z };
								ImGui::ColorEdit3(pProp.Name.c_str(), f);

								*p = glm::make_vec3(f);
								break;
							}

							case Lamp::PropertyType::Color4:
							{
								glm::vec4* p = static_cast<glm::vec4*>(pProp.Value);

								float f[4] = { p->x, p->y, p->z, p->w };
								ImGui::ColorEdit4(pProp.Name.c_str(), f);

								*p = glm::make_vec4(f);
								break;
							}
							}
						}
					}
				}

				if (ImGui::Button("Add Component"))
				{
					m_AddComponentOpen = !m_AddComponentOpen;
				}
			}
			else if (auto pBrush = dynamic_cast<Lamp::Brush*>(m_pSelectedObject))
			{
				ImGui::Text("Brush");

				std::string name = pBrush->GetName();
				//ImGui::InputText("Name", &name);
				pBrush->SetName(name);

				if (ImGui::CollapsingHeader("Transform"))
				{
					glm::vec3 pos = pBrush->GetPosition();
					float f[3] = { pos.x, pos.y, pos.z };

					ImGui::InputFloat3("Position", f);
					pBrush->SetPosition(glm::make_vec3(f));

					glm::vec3 rot = pBrush->GetRotation();
					float r[3] = { rot.x, rot.y, rot.z };

					ImGui::InputFloat3("Rotation", r);
					pBrush->SetRotation(glm::make_vec3(r));

					glm::vec3 scale = pBrush->GetScale();
					float s[3] = { scale.x, scale.y, scale.z };

					ImGui::InputFloat3("Scale", s);
					pBrush->SetScale(glm::make_vec3(s));
				}

				if (ImGui::CollapsingHeader("Physics"))
				{
					glm::vec3 vel = pBrush->GetPhysicalEntity()->GetVelocity();
					float f[3] = { vel.x, vel.y, vel.z };

					ImGui::InputFloat3("Velocity", f);
					pBrush->GetPhysicalEntity()->SetVelocity(glm::make_vec3(f));

					float m = pBrush->GetPhysicalEntity()->GetMass();
					ImGui::InputFloat("Mass", &m);
					pBrush->GetPhysicalEntity()->SetMass(m);
				}
			}
		}
		ImGui::End();
	}

	void Sandbox3D::UpdateModelImporter()
	{
		if (!m_ModelImporterOpen)
		{
			return;
		}

		if (m_pShader == nullptr)
		{
			m_pShader = Lamp::Shader::Create("engine/shaders/3d/shader_vs.glsl", "engine/shaders/3d/shader_fs.glsl");
		}

		ImGui::Begin("Model importer", &m_ModelImporterOpen);

		static std::string path = "";
		static std::string savePath = "";

		if (ImGui::Button("Load##fbx"))
		{
			path = Lamp::FileSystem::GetFileFromDialogue();
			if (path != "" && std::filesystem::exists(path))
			{
				m_pModelToImport = Lamp::GeometrySystem::ImportModel(path);

				savePath = path.substr(0, path.find_last_of('.'));
				savePath += ".lgf";

				m_pModelToImport->GetMaterial().SetShader(m_pShader);

				m_pModelToImport->GetMaterial().SetDiffuse(Lamp::Texture2D::Create("engine/textures/default/defaultTexture.png"));
				m_pModelToImport->GetMaterial().SetSpecular(Lamp::Texture2D::Create("engine/textures/default/defaultTexture.png"));
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			savePath = Lamp::FileSystem::SaveFileInDialogue();
			Lamp::GeometrySystem::SaveToPath(m_pModelToImport, savePath);
			path = "";
			savePath = "";
			m_pModelToImport = nullptr;
		}
		ImGui::Text(("Source path: " + path).c_str());
		ImGui::Text(("Destination path: " + savePath).c_str());

		if (m_pModelToImport != nullptr)
		{
			static std::string diffPath = m_pModelToImport->GetMaterial().GetDiffuse()->GetPath();
			//ImGui::InputText("Diffuse path:", &diffPath);
			ImGui::SameLine();
			if (ImGui::Button("Load##diff"))
			{
				diffPath = Lamp::FileSystem::GetFileFromDialogue();
			}
			m_pModelToImport->GetMaterial().SetDiffuse(Lamp::Texture2D::Create(diffPath));

			static std::string specPath = m_pModelToImport->GetMaterial().GetSpecular()->GetPath();
			//ImGui::InputText("Specular path:", &specPath);
			ImGui::SameLine();
			if (ImGui::Button("Load##spec"))
			{
				specPath = Lamp::FileSystem::GetFileFromDialogue();
			}
			m_pModelToImport->GetMaterial().SetSpecular(Lamp::Texture2D::Create(specPath));

			//ImGui::InputText("Shader path:", &m_pModelToImport->GetMaterial().GetShader()->GetVertexPath());
		}

		ImGui::End();
	}

	void Sandbox3D::UpdateLayerView()
	{
		if (!m_LayerViewOpen)
		{
			return;
		}

		ImGui::Begin("Layers", &m_LayerViewOpen);

		int startId = 0;
		int j = 0;

		for (Lamp::ObjectLayer& layer : Lamp::ObjectLayerManager::Get()->GetLayers())
		{
			if (ImGui::Button(std::string("A##" + std::to_string(j)).c_str()))
			{
				layer.IsActive = !layer.IsActive;
			}

			ImGui::SameLine();
			if (ImGui::TreeNode(layer.Name.c_str()))
			{

				for (int i = 0; i < layer.Objects.size(); i++)
				{
					if (ImGui::Button(std::string("A##" + std::to_string(i)).c_str()))
					{
						layer.Objects[i]->SetIsActive(!layer.Objects[i]->GetIsActive());
					}

					ImGui::SameLine();

					startId++;
					ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					ImGui::TreeNodeEx((void*)(intptr_t)startId, nodeFlags, layer.Objects[i]->GetName().c_str());

					if (ImGui::IsItemClicked())
					{
						m_pSelectedObject = layer.Objects[i];
					}
				}

				ImGui::TreePop();
			}
			j++;
		}

		ImGui::End();
	}

	void Sandbox3D::UpdateAddComponent()
	{
		if (!m_AddComponentOpen)
		{
			return;
		}

		ImGui::Begin("Add Component", &m_AddComponentOpen);
		
		static std::string selected = "";

		for (auto& key : Lamp::ComponentRegistry::s_Methods())
		{
			if (ImGui::Selectable(key.first.c_str()))
			{
				selected = key.first;
			}
		}

		if (ImGui::Button("Close"))
		{
			m_AddComponentOpen = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Add"))
		{
			if (selected != "")
			{
				if (auto pEnt = dynamic_cast<Lamp::Entity*>(m_pSelectedObject))
				{
					if (pEnt->HasComponent(selected))
					{
						m_AddComponentOpen = false;
						LP_ERROR("Entity already has component!");
					}

					Ref<Lamp::EntityComponent> comp = Lamp::ComponentRegistry::Create(selected);
					if (comp != nullptr)
					{
						if (pEnt->AddComponent(comp))
						{
							m_AddComponentOpen = false;
						}
					}
				}
			}
		}

		ImGui::End();
	}

	void Sandbox3D::UpdateCreateTool()
	{
		if (!m_CreateToolOpen)
		{
			return;
		}

		ImGui::Begin("Create", &m_CreateToolOpen);

		static bool brushListOpen = false;
		
		if (!brushListOpen)
		{
			if (ImGui::Button("Entity"))
			{
				Lamp::Entity* pEnt = Lamp::EntityManager::Get()->Create();
				pEnt->SetPosition(glm::vec3(0.f, 0.f, 0.f));

				m_pSelectedObject = pEnt;
			}
			
			ImGui::SameLine();

			if (ImGui::Button("Brush"))
			{
				brushListOpen = !brushListOpen;
			}
		}

		if (brushListOpen)
		{
			if (ImGui::Button("Back##0"))
			{
				brushListOpen = false;
			}

			ImGui::SameLine();

			if (ImGui::Button("Create"))
			{
				if (m_SelectedFile.GetFileType() == Lamp::FileType::Brush)
				{
					m_pSelectedObject = Lamp::Brush::Create(m_SelectedFile.GetPath());
				}
			}

			if (ImGui::BeginChild("Brushes"))
			{
				std::vector<std::string> folders = Lamp::FileSystem::GetAssetFolders();
				Lamp::FileSystem::PrintBrushes(folders);

			}
			ImGui::EndChild();
		}

		ImGui::End();
	}

	void Sandbox3D::UpdateLogTool()
	{
		static bool autoScroll = true;

		if (!m_LogToolOpen)
		{
			return;
		}

		ImGui::Begin("Log", &m_LogToolOpen);
		
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &autoScroll);
			ImGui::EndPopup();
		}

		if (ImGui::Button("Clear"))
		{
			Lamp::Log::GetCoreLogger()->GetMessages().clear();
		}
		ImGui::SameLine();
		if (ImGui::Button("Options"))
		{
			ImGui::OpenPopup("Options");
		}

		ImGui::Separator();

		ImGui::BeginChild("text");

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		for (auto& msg : Lamp::Log::GetCoreLogger()->GetMessages())
		{
			if (msg.Level == Lamp::LogLevel::Trace)
			{
				ImGui::TextColored(ImVec4(212.f / 255.f, 212.f / 255.f, 212.f / 255.f, 1.f), msg.Msg.c_str());
			}
			else if (msg.Level == Lamp::LogLevel::Info)
			{
				ImGui::Text(msg.Msg.c_str());
			}
			else if (msg.Level == Lamp::LogLevel::Warn)
			{
				ImGui::TextColored(ImVec4(255.f / 255.f, 235.f / 255.f, 54.f / 255.f, 1.f), msg.Msg.c_str());
			}
			else if (msg.Level == Lamp::LogLevel::Error)
			{
				ImGui::TextColored(ImVec4(255.f / 255.f, 0, 0, 1.f), msg.Msg.c_str());
			}
			else if (msg.Level == Lamp::LogLevel::Critical)
			{
				ImGui::TextColored(ImVec4(255.f / 255.f, 0, 0, 1.f), msg.Msg.c_str());
			}
		}
		ImGui::PopStyleVar();

		if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.f);
		}
		ImGui::EndChild();
		ImGui::End();
	}

	void Sandbox3D::CreateDockspace()
	{
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		bool pp = true;
		bool* p = &pp;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", p, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}


		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save Level"))
				{
					Lamp::LevelSystem::SaveLevel("assets/levels/" + Lamp::LevelSystem::GetCurrentLevel()->GetName() + ".level", Lamp::LevelSystem::GetCurrentLevel());
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("Import Model", NULL, &m_ModelImporterOpen);
				ImGui::MenuItem("Properties", NULL, &m_InspectiorOpen);
				ImGui::MenuItem("Asset browser", NULL, &m_AssetBrowserOpen);
				ImGui::MenuItem("Layer view", NULL, &m_LayerViewOpen);
				ImGui::MenuItem("Log", NULL, &m_LogToolOpen);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Play"))
			{
				ImGui::MenuItem("Play", "CTRL + G", &m_ShouldPlay);
				ImGui::MenuItem("Play physics", NULL, &m_ShouldPlayPhysics);

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();
	}

	bool Sandbox3D::OnMouseMoved(Lamp::MouseMovedEvent& e)
	{
		m_MouseHoverPos = glm::vec2(e.GetX(), e.GetY());

		return true;
	}
}