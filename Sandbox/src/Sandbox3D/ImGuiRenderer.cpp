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
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>

#include <Lamp/Math/Math.h>
#include "Windows/ModelImporter.h"
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <Lamp/Meshes/Materials/MaterialLibrary.h>

#include <Lamp/Core/Application.h>

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
			m_PerspectiveHover = ImGui::IsWindowHovered() && ImGui::IsWindowFocused() || !m_ModelImporter->GetIsOpen() && ImGui::IsWindowHovered();
			m_SandboxController->GetCameraController()->SetControlsEnabled(m_PerspectiveHover);

			ImVec2 perspectivePanelSize = ImGui::GetContentRegionAvail();
			if (m_PerspectiveSize != *((glm::vec2*) & perspectivePanelSize))
			{
				Lamp::Renderer3D::GetFrameBuffer()->Update((uint32_t)perspectivePanelSize.x, (uint32_t)perspectivePanelSize.y);
				m_PerspectiveSize = { perspectivePanelSize.x, perspectivePanelSize.y };

				m_SandboxController->GetCameraController()->UpdateProjection((uint32_t)perspectivePanelSize.x, (uint32_t)perspectivePanelSize.y);
			}

			uint32_t textureID = Lamp::Renderer3D::GetFrameBuffer()->GetColorAttachment();
			ImGui::Image((void*)(uint64_t)textureID, ImVec2{ m_PerspectiveSize.x, m_PerspectiveSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

			std::string frameInfo = "FrameTime: " + std::to_string(Lamp::Application::Get().GetFrameTime().GetFrameTime()) + ". FPS: " + std::to_string(Lamp::Application::Get().GetFrameTime().GetFramesPerSecond()) + ". Using VSync: " + std::to_string(Lamp::Application::Get().GetWindow().GetIsVSync());
			ImGui::SetCursorPos(ImVec2(20, 40));
			ImGui::Text(frameInfo.c_str());
		}

		//Guizmos
		static glm::mat4 transform = glm::mat4(1.f);
		static glm::mat4 lastTrans = glm::mat4(1.f);
		static bool beginMove = false;
		static bool hasStarted = false;
		static bool firstTime = true;

		if (m_pSelectedObject)
		{
			transform = m_pSelectedObject->GetModelMatrix();

			// TODO: improve this first part.
			if (firstTime)
			{
				lastTrans = transform;
				firstTime = false;
			}

			if (ImGuizmo::IsDragging() && !beginMove)
			{
				beginMove = true;
			}

			if (!ImGuizmo::IsDragging())
			{
				hasStarted = false;
			}

			if (beginMove && !hasStarted)
			{
				lastTrans = m_pSelectedObject->GetModelMatrix();
				beginMove = false;
				hasStarted = true;
			}
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			
			ImGuizmo::SetRect(perspectivePos.x, perspectivePos.y, m_PerspectiveSize.x, m_PerspectiveSize.y);
			ImGuizmo::Manipulate(glm::value_ptr(m_SandboxController->GetCameraController()->GetCamera()->GetViewMatrix()),
				glm::value_ptr(m_SandboxController->GetCameraController()->GetCamera()->GetProjectionMatrix()),
				m_ImGuizmoOperation, ImGuizmo::LOCAL, glm::value_ptr(transform));

			glm::vec3 p, r, s;
			Lamp::Math::DecomposeTransform(transform, p, r, s);

			r = r - m_pSelectedObject->GetRotation();

			m_pSelectedObject->SetPosition(p);
			m_pSelectedObject->AddRotation(r);
			m_pSelectedObject->SetScale(s);

			if (m_pSelectedObject->GetModelMatrix() != lastTrans && !ImGuizmo::IsDragging())
			{
				Command cmd;
				cmd.cmd = Cmd::Transform;
				cmd.lastData = new glm::mat4(lastTrans);
				cmd.object = m_pSelectedObject;

				m_PerspecticeCommands.push_front(cmd);
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
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
			bool perspHover = false;

			ImGui::Begin("Perspective");
			{
				windowPos = glm::vec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
				windowSize = glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
				perspHover = ImGui::IsWindowHovered();
			}
			ImGui::End();

			if (m_MousePressed && perspHover && !ImGuizmo::IsOver())
			{
				mousePos -= windowPos;

				Command cmd;
				cmd.cmd = Cmd::Selection;
				cmd.lastData = m_pSelectedObject;
				cmd.object = nullptr;

				m_pSelectedObject = Lamp::ObjectLayerManager::Get()->GetObjectFromPoint(m_SandboxController->GetCameraController()->ScreenToWorldCoords(mousePos, m_PerspectiveSize), m_SandboxController->GetCameraController()->GetPosition());
			
				if (cmd.lastData != m_pSelectedObject)
				{
					m_PerspecticeCommands.push_front(cmd);
				}
			}

			if (auto pEnt = dynamic_cast<Lamp::Entity*>(m_pSelectedObject))
			{
				ImGui::Text("Entity");

				std::string name = pEnt->GetName();
				ImGui::InputText("Name", &name);
				pEnt->SetName(name);

				if (ImGui::CollapsingHeader("Transform"))
				{
					glm::vec3 pos = pEnt->GetPosition();
					float f[3] = { pos.x, pos.y, pos.z };

					ImGui::InputFloat3("Position", f);
					pEnt->SetPosition(glm::make_vec3(f));

					if (pos != pEnt->GetPosition())
					{
						Command cmd;
						cmd.cmd = Cmd::Position;
						//cmd.lastData = 
					}

					glm::vec3 rot = pEnt->GetRotation();
					float r[3] = { rot.x, rot.y, rot.z };

					ImGui::InputFloat3("Rotation", r);
					pEnt->SetRotation(glm::make_vec3(r));

					glm::vec3 scale = pEnt->GetScale();
					float s[3] = { scale.x, scale.y, scale.z };

					ImGui::InputFloat3("Scale", s);
					pEnt->SetScale(glm::make_vec3(s));
				}

				for (size_t i = 0; i < pEnt->GetComponents().size(); i++)
				{
					if (DrawComponent(pEnt->GetComponents()[i].get()))
					{
						pEnt->RemoveComponent(pEnt->GetComponents()[i]);
						i--;
					}
				}
			}
			else if (auto pBrush = dynamic_cast<Lamp::Brush*>(m_pSelectedObject))
			{
				ImGui::Text("Brush");

				std::string name = pBrush->GetName();
				ImGui::InputText("Name", &name);
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

				if (ImGui::CollapsingHeader("Material"))
				{
					std::string n = pBrush->GetModel()->GetMaterial().GetName();
					ImGui::InputText("Name##mat", &n);

					if (n != pBrush->GetModel()->GetMaterial().GetName())
					{
						if (auto mat = Lamp::MaterialLibrary::GetMaterial(n); mat.GetIndex() != -1)
						{
							pBrush->GetModel()->SetMaterial(Lamp::MaterialLibrary::GetMaterial(n));
						}
					}
				}
			}

			if (auto Ent = dynamic_cast<Lamp::Entity*>(m_pSelectedObject))
			{
				if (ImGui::Button("Add Component"))
				{
					ImGui::OpenPopup("AddComponent");
				}
			}

			UpdateAddComponent();
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
		if (ImGui::BeginPopup("AddComponent"))
		{
			for (auto& key : Lamp::ComponentRegistry::s_Methods())
			{
				if (ImGui::MenuItem(key.first.c_str()))
				{
					if (auto pEnt = dynamic_cast<Lamp::Entity*>(m_pSelectedObject))
					{
						if (pEnt->HasComponent(key.first.c_str()))
						{
							m_AddComponentOpen = false;
							LP_CORE_WARN("Entity already has component!");
						}

						Ref<Lamp::EntityComponent> comp = Lamp::ComponentRegistry::Create(key.first.c_str());
						if (comp != nullptr)
						{
							pEnt->AddComponent(comp);
							
							Lamp::EntityComponentAddedEvent e;
							pEnt->OnEvent(e);
						}
					}

					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}
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
				m_pSelectedObject = Lamp::EntityManager::Get()->Create();
				m_pSelectedObject->SetPosition(glm::vec3(0.f, 0.f, 0.f));
				static_cast<Lamp::Entity*>(m_pSelectedObject)->SetSaveable(true);
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

	bool Sandbox3D::DrawComponent(Lamp::EntityComponent* ptr)
	{
		bool removeComp = false;

		if (ImGui::CollapsingHeader(ptr->GetName().c_str()))
		{
			if (ImGui::Button("Remove"))
			{
				removeComp = true;
			}

			for (auto& pProp : ptr->GetComponentProperties().GetProperties())
			{
				switch (pProp.PropertyType)
				{
					case Lamp::PropertyType::Int:
					{
						int* p = static_cast<int*>(pProp.Value);
						int v = *p;
						
						ImGui::InputInt(pProp.Name.c_str(), &v);
						if (v != *p)
						{
							*p = v;

							Lamp::EntityPropertyChangedEvent e;
							ptr->GetOwner()->OnEvent(e);
						}

						break;
					}

					case Lamp::PropertyType::Bool:
					{
						bool* p = static_cast<bool*>(pProp.Value);
						bool v = *p;

						ImGui::Checkbox(pProp.Name.c_str(), &v);
						if (v != *p)
						{
							*p = v;

							Lamp::EntityPropertyChangedEvent e;
							ptr->GetOwner()->OnEvent(e);
						}
						
						break;
					}

					case Lamp::PropertyType::Float:
					{
						float* p = static_cast<float*>(pProp.Value);
						float v = *p;

						ImGui::InputFloat(pProp.Name.c_str(), &v);
						if (v != *p)
						{
							*p = v;

							Lamp::EntityPropertyChangedEvent e;
							ptr->GetOwner()->OnEvent(e);
						}
						break;
					}

					case Lamp::PropertyType::Float2:
					{
						glm::vec2* p = static_cast<glm::vec2*>(pProp.Value);
						glm::vec2 v = *p;
						
						ImGui::InputFloat2(pProp.Name.c_str(), glm::value_ptr(v), 3);
						if (v != *p)
						{
							*p = v;

							Lamp::EntityPropertyChangedEvent e;
							ptr->GetOwner()->OnEvent(e);
						}
						break;
					}

					case Lamp::PropertyType::Float3:
					{
						glm::vec3* p = static_cast<glm::vec3*>(pProp.Value);
						glm::vec3 v = *p;

						ImGui::InputFloat3(pProp.Name.c_str(), glm::value_ptr(v), 3);
						if (v != *p)
						{
							*p = v;

							Lamp::EntityPropertyChangedEvent e;
							ptr->GetOwner()->OnEvent(e);
						}
						break;
					}

					case Lamp::PropertyType::Float4:
					{
						glm::vec4* p = static_cast<glm::vec4*>(pProp.Value);
						glm::vec4 v = *p;

						ImGui::InputFloat4(pProp.Name.c_str(), glm::value_ptr(*p), 3);
						if (v != *p)
						{
							*p = v;

							Lamp::EntityPropertyChangedEvent e;
							ptr->GetOwner()->OnEvent(e);
						}
						break;
					}

					case Lamp::PropertyType::String:
					{
						std::string* s = static_cast<std::string*>(pProp.Value);
						std::string v = *s;

						ImGui::InputText(pProp.Name.c_str(), &v);
						if (v != *s)
						{
							*s = v;

							Lamp::EntityPropertyChangedEvent e;
							ptr->GetOwner()->OnEvent(e);
						}
						break;
					}

					case Lamp::PropertyType::Path:
					{
						std::string* s = static_cast<std::string*>(pProp.Value);
						std::string v = *s;

						ImGui::InputText(pProp.Name.c_str(), &v);
						ImGui::SameLine();
						if (ImGui::Button("Open..."))
						{
							std::string path = Lamp::FileDialogs::OpenFile("All (*.*)\0*.*\0");
							if (!path.empty())
							{
								v = path;
							}
						}

						if (v != *s)
						{
							*s = v;

							Lamp::EntityPropertyChangedEvent e;
							ptr->GetOwner()->OnEvent(e);
						}
						break;
					}

					case Lamp::PropertyType::Color3:
					{
						glm::vec3* p = static_cast<glm::vec3*>(pProp.Value);
						glm::vec3 v = *p;

						ImGui::ColorEdit3(pProp.Name.c_str(), glm::value_ptr(v));
						if (v != *p)
						{
							*p = v;

							Lamp::EntityPropertyChangedEvent e;
							ptr->GetOwner()->OnEvent(e);
						}
						break;
					}

					case Lamp::PropertyType::Color4:
					{
						glm::vec4* p = static_cast<glm::vec4*>(pProp.Value);
						glm::vec4 v = *p;

						ImGui::ColorEdit4(pProp.Name.c_str(), glm::value_ptr(v));
						if (v != *p)
						{
							*p = v;

							Lamp::EntityPropertyChangedEvent e;
							ptr->GetOwner()->OnEvent(e);
						}
						break;
					}
				}
			}
		}

		return removeComp;
	}

	void Sandbox3D::UpdateLevelSettings()
	{
		if (!m_LevelSettingsOpen)
		{
			return;
		}

		ImGui::Begin("Level Settings", &m_LevelSettingsOpen);

		ImGui::ColorEdit3("Global Ambient", glm::value_ptr(Lamp::LevelSystem::GetEnvironment().GlobalAmbient));

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
				if (ImGui::MenuItem("New", "Ctrl + N"))
				{
					NewLevel();
				}

				if (ImGui::MenuItem("Open...", "Ctrl + O"))
				{
					OpenLevel();
				}

				if (ImGui::MenuItem("Save As...", "Ctrl + Shift + S"))
				{
					SaveLevelAs();
				}

				if (ImGui::MenuItem("Save", "Ctrl + S"))
				{
					if (Lamp::LevelSystem::GetCurrentLevel()->GetPath().empty())
					{
						SaveLevelAs();
					}
					else
					{
						Lamp::LevelSystem::SaveLevel(Lamp::LevelSystem::GetCurrentLevel());
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl + Z"))
				{
					Undo();
				}

				if (ImGui::MenuItem("Redo", "Ctrl + Y"))
				{
					Redo();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("Import Model", NULL, &m_ModelImporter->GetIsOpen());
				ImGui::MenuItem("Properties", NULL, &m_InspectiorOpen);
				ImGui::MenuItem("Asset browser", NULL, &m_AssetBrowserOpen);
				ImGui::MenuItem("Layer view", NULL, &m_LayerViewOpen);
				ImGui::MenuItem("Log", NULL, &m_LogToolOpen);
				ImGui::MenuItem("Level Settings", NULL, &m_LevelSettingsOpen);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Play"))
			{
				ImGui::MenuItem("Play", "Ctrl + G", &m_ShouldPlay);
				ImGui::MenuItem("Play physics", NULL, &Lamp::Application::Get().GetIsSimulating());

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