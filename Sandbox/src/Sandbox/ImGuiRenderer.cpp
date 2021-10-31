#include "lppch.h"
#include "Sandbox.h"

#include <imgui/misc/cpp/imgui_stdlib.h>

#include <Lamp/Objects/Entity/Base/Entity.h>
#include <Lamp/Objects/Entity/Base/ComponentRegistry.h>
#include <ImGuizmo/ImGuizmo.h>

#include <Lamp/Core/CoreLogger.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>

#include <Lamp/Math/Math.h>
#include "Windows/MeshImporterPanel.h"
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <Lamp/Mesh/Materials/MaterialLibrary.h>

#include <Lamp/Core/Application.h>
#include <Lamp/GraphKey/GraphKeyGraph.h>
#include "Sandbox/Windows/GraphKey.h"

#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Rendering/RenderGraph/RenderGraph.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodeEnd.h>
#include <Lamp/Rendering/Vertices/FrameBuffer.h>

#include <Lamp/Utility/UIUtility.h>

namespace Sandbox
{
	using namespace Lamp;

	void Sandbox::UpdatePerspective()
	{
		LP_PROFILE_FUNCTION();

		if (!m_PerspectiveOpen)
		{
			return;
		}

		glm::vec2 perspectivePos;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f });
		ImGui::Begin("Perspective");
		{
			perspectivePos = glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
			m_PerspectiveHover = ImGui::IsWindowHovered();
			m_SandboxController->GetCameraController()->SetControlsEnabled(m_PerspectiveHover);
			m_PerspectiveFocused = ImGui::IsWindowFocused();

			//Viewport bounds
			auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
			auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			auto viewportOffset = ImGui::GetWindowPos();

			m_PerspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
			m_PerspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

			ImVec2 perspectivePanelSize = ImGui::GetContentRegionAvail();
			if (m_PerspectiveSize != *((glm::vec2*)&perspectivePanelSize))
			{
				m_PerspectiveSize = { perspectivePanelSize.x, perspectivePanelSize.y };

				Lamp::EditorViewportSizeChangedEvent e((uint32_t)perspectivePanelSize.x, (uint32_t)perspectivePanelSize.y);
				OnEvent(e);
				g_pEnv->pLevel->OnEvent(e);
			}

			uint32_t textureID = 0;
			if (Renderer::GetRenderGraph())
			{
				auto& renderGraph = Renderer::GetRenderGraph();

				if (renderGraph->GetSpecification().endNode->framebuffer)
				{
					textureID = renderGraph->GetSpecification().endNode->framebuffer->GetColorAttachmentID(0);
					m_SelectionBuffer = renderGraph->GetSpecification().endNode->framebuffer;
				}
			}
			ImGui::Image((void*)(uint64_t)textureID, ImVec2{ m_PerspectiveSize.x, m_PerspectiveSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

			if (auto ptr = UI::DragDropTarget({ "CONTENT_BROWSER_ITEM", "BRUSH_ITEM" }))
			{
				const wchar_t* wPath = (const wchar_t*)ptr;
				std::filesystem::path path(wPath);

				AssetType type = g_pEnv->pAssetManager->GetAssetTypeFromPath(path);
				if (type == Lamp::AssetType::Level)
				{
					OpenLevel(path);
				}

				if (type == Lamp::AssetType::Mesh)
				{
					m_pSelectedObject = Lamp::Brush::Create(path.string());
				}
			}
		}

		//Guizmo
		static glm::mat4 transform = glm::mat4(1.f);

		if (m_pSelectedObject && m_SceneState != SceneState::Play)
		{
			transform = m_pSelectedObject->GetTransform();

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_PerspectiveBounds[0].x, m_PerspectiveBounds[0].y, m_PerspectiveBounds[1].x - m_PerspectiveBounds[0].x, m_PerspectiveBounds[1].y - m_PerspectiveBounds[0].y);

			bool snap = Lamp::Input::IsKeyPressed(LP_KEY_LEFT_CONTROL);
			bool duplicate = Lamp::Input::IsKeyPressed(LP_KEY_LEFT_SHIFT);

			float snapValue = 0.5f;
			if (m_ImGuizmoOperation == ImGuizmo::ROTATE)
			{
				snapValue = 45.f;
			}

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(
				glm::value_ptr(m_SandboxController->GetCameraController()->GetCamera()->GetViewMatrix()),
				glm::value_ptr(m_SandboxController->GetCameraController()->GetCamera()->GetProjectionMatrix()),
				m_ImGuizmoOperation, ImGuizmo::WORLD, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			static bool hasDuplicated = false;

			if (ImGuizmo::IsUsing())
			{
				if (duplicate && !hasDuplicated)
				{
					if (typeid(*m_pSelectedObject) == typeid(Lamp::Brush))
					{
						m_pSelectedObject->SetIsSelected(false);

						auto brush = static_cast<Lamp::Brush*>(m_pSelectedObject);
						m_pSelectedObject = Lamp::Brush::Duplicate(brush, true);

						m_pSelectedObject->SetIsSelected(true);
					}
					else if (typeid(*m_pSelectedObject) == typeid(Lamp::Entity))
					{
						m_pSelectedObject->SetIsSelected(false);

						auto entity = static_cast<Lamp::Entity*>(m_pSelectedObject);
						m_pSelectedObject = Lamp::Entity::Duplicate(entity, true);

						m_pSelectedObject->SetIsSelected(true);
					}

					hasDuplicated = true;
				}
				else if (transform != m_pSelectedObject->GetTransform())
				{
					glm::vec3 p, r, s;
					Math::DecomposeTransform(transform, p, r, s);

					glm::vec3 deltaRot = r - m_pSelectedObject->GetRotation();
					m_pSelectedObject->SetPosition(p);
					m_pSelectedObject->AddRotation(deltaRot);
					m_pSelectedObject->SetScale(s);
				}
			}
			else
			{
				hasDuplicated = false;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar(2);
	}

	void Sandbox::UpdateProperties()
	{
		LP_PROFILE_FUNCTION();

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

			if (m_MousePressed && perspHover && !ImGuizmo::IsOver() && m_SceneState != SceneState::Play)
			{
				mousePos -= windowPos;

				/////Mouse picking/////

				auto [mx, my] = ImGui::GetMousePos();
				mx -= m_PerspectiveBounds[0].x;
				my -= m_PerspectiveBounds[0].y;

				glm::vec2 perspectiveSize = m_PerspectiveBounds[1] - m_PerspectiveBounds[0];
				my = perspectiveSize.y - my;

				int mouseX = (int)mx;
				int mouseY = (int)my;

				if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)perspectiveSize.x && mouseY < (int)perspectiveSize.y)
				{
					int pixelData = m_SelectionBuffer->ReadPixel(1, mouseX, mouseY);

					Object* newSelected = Lamp::Entity::Get(pixelData);
					if (!newSelected)
					{
						newSelected = Lamp::Brush::Get(pixelData);
					}

					if (newSelected && !newSelected->GetIsFrozen())
					{
						if (m_pSelectedObject)
						{
							m_pSelectedObject->SetIsSelected(false);
						}

						newSelected->SetIsSelected(true);
						m_pSelectedObject = newSelected;
					}
					else
					{
						if (m_pSelectedObject)
						{
							m_pSelectedObject->SetIsSelected(false);
						}
						m_pSelectedObject = nullptr;
					}
				}

				m_SelectionBuffer->Unbind();
				////////////////////////
			}

			if (auto pEnt = dynamic_cast<Lamp::Entity*>(m_pSelectedObject))
			{
				ImGui::Text("Entity");

				std::string name = pEnt->GetName();
				UI::InputText("Name", name);
				pEnt->SetName(name);

				if (UI::TreeNodeFramed("Transform"))
				{
					UI::BeginProperties("transProps");

					UI::PropertyAxisColor("Position", const_cast<glm::vec3&>(pEnt->GetPosition()));

					glm::vec3 rot = pEnt->GetRotation();
					rot = glm::degrees(rot);
					if (UI::PropertyAxisColor("Rotation", rot))
					{
						pEnt->SetRotation(glm::radians(rot));
					}

					UI::PropertyAxisColor("Scale", const_cast<glm::vec3&>(pEnt->GetScale()), 1.f);

					UI::EndProperties();
					UI::TreeNodePop();
				}

				std::string graphButtonString = pEnt->GetGraphKeyGraph() ? "Open Graph" : "Create Graph";
				if (ImGui::Button(graphButtonString.c_str()))
				{
					if (!pEnt->GetGraphKeyGraph())
					{
						Lamp::GraphKeyGraphSpecification spec;
						spec.name = pEnt->GetName();
						spec.path = "Assets/libs/graphkey/" + spec.name + ".graphkey";
						pEnt->SetGraphKeyGraph(CreateRef<Lamp::GraphKeyGraph>(spec));

						if (auto vs = (GraphKey*)(m_pWindows[1]))
						{
							vs->SetIsOpen(true);
							vs->SetCurrentlyOpenGraph(pEnt->GetGraphKeyGraph(), pEnt->GetID());
						}
					}
					else
					{
						if (auto vs = (GraphKey*)(m_pWindows[1]))
						{
							vs->SetIsOpen(true);
							vs->SetCurrentlyOpenGraph(pEnt->GetGraphKeyGraph(), pEnt->GetID());
						}
					}
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
				UI::InputText("Name", name);
				pBrush->SetName(name);

				if (UI::TreeNodeFramed("Transform"))
				{
					UI::BeginProperties("transProp");

					UI::PropertyAxisColor("Position", const_cast<glm::vec3&>(pBrush->GetPosition()));

					glm::vec3 rot = pBrush->GetRotation();
					rot = glm::degrees(rot);
					if (UI::PropertyAxisColor("Rotation", rot))
					{
						pBrush->SetRotation(glm::radians(rot));
					}

					UI::PropertyAxisColor("Scale", const_cast<glm::vec3&>(pBrush->GetScale()));

					UI::EndProperties();
					UI::TreeNodePop();
				}

				if (UI::TreeNodeFramed("Materials"))
				{
					int i = 0;
					for (auto& mat : pBrush->GetModel()->GetMaterials())
					{
						std::string input = mat.second->GetName();

						UI::BeginProperties();

						UI::Property("Material" + std::to_string(i), input);
						i++;

						UI::EndProperties();
					}

					UI::TreeNodePop();
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

	void Sandbox::UpdateAddComponent()
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

	void Sandbox::UpdateCreateTool()
	{
		LP_PROFILE_FUNCTION();

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
				m_pSelectedObject = Lamp::Entity::Create();
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

			}	/*if (m_SelectedFile.GetFileType() == Lamp::FileType::Brush)
				{
					m_pSelectedObject = Lamp::Brush::Create(m_SelectedFile.GetPath());
				}*/

			if (ImGui::BeginChild("Brushes"))
			{
				std::vector<std::string> folders = Lamp::FileSystem::GetAssetFolders();
				Lamp::FileSystem::PrintBrushes(folders);

			}
			ImGui::EndChild();
		}

		ImGui::End();
	}

	void Sandbox::UpdateLogTool()
	{
		LP_PROFILE_FUNCTION();

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
		for (const auto& msg : Lamp::Log::GetCoreLogger()->GetMessages())
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

	bool Sandbox::DrawComponent(Lamp::EntityComponent* ptr)
	{
		LP_PROFILE_FUNCTION();

		bool removeComp = false;
		bool open = UI::TreeNodeFramed(ptr->GetName(), true, 0.f, { 0.f, 2.f });

		float buttonSize = 20.f + GImGui->Style.FramePadding.y * 0.5f;
		float availRegion = ImGui::GetContentRegionAvail().x;

		if (!open)
		{
			UI::SameLine(availRegion - buttonSize * 0.5f + GImGui->Style.FramePadding.x * 0.5f);
		}
		else
		{
			UI::SameLine(availRegion + buttonSize * 0.5f + GImGui->Style.FramePadding.x * 0.5f);
		}
		std::string id = "-###Remove" + ptr->GetName();

		{
			UI::ScopedStyleFloat round{ ImGuiStyleVar_FrameRounding, 0.f };
			UI::ScopedStyleFloat2 pad{ ImGuiStyleVar_FramePadding, { 0.f, 0.f } };

			if (ImGui::Button(id.c_str(), ImVec2{ buttonSize, buttonSize }))
			{
				removeComp = true;
			}
		}

		if (open)
		{
			UI::BeginProperties("compProp" + ptr->GetName());

			for (auto& prop : ptr->GetComponentProperties().GetProperties())
			{
				bool propertyChanged = false;

				switch (prop.propertyType)
				{
					case Lamp::PropertyType::Int: propertyChanged = UI::Property(prop.name, *static_cast<int*>(prop.value)); break;
					case Lamp::PropertyType::Bool: propertyChanged = UI::Property(prop.name, *static_cast<bool*>(prop.value)); break;
					case Lamp::PropertyType::Float: propertyChanged = UI::Property(prop.name, *static_cast<float*>(prop.value)); break;
					case Lamp::PropertyType::Float2: propertyChanged = UI::Property(prop.name, *static_cast<glm::vec2*>(prop.value)); break;
					case Lamp::PropertyType::Float3: propertyChanged = UI::Property(prop.name, *static_cast<glm::vec3*>(prop.value)); break;
					case Lamp::PropertyType::Float4: propertyChanged = UI::Property(prop.name, *static_cast<glm::vec4*>(prop.value)); break;
					case Lamp::PropertyType::String: propertyChanged = UI::Property(prop.name, *static_cast<std::string*>(prop.value)); break;
					case Lamp::PropertyType::Path: propertyChanged = UI::Property(prop.name, std::filesystem::path(*static_cast<std::string*>(prop.value))); break;
					case Lamp::PropertyType::Color3: propertyChanged = UI::Property(prop.name, *static_cast<glm::vec3*>(prop.value), false); break;
					case Lamp::PropertyType::Color4: propertyChanged = UI::Property(prop.name, *static_cast<glm::vec4*>(prop.value), true); break;
				}

				if (propertyChanged)
				{
					Lamp::ObjectPropertyChangedEvent e;
					ptr->GetEntity()->OnEvent(e);
				}
			}

			UI::EndProperties();
			UI::TreeNodePop();
		}

		return removeComp;
	}

	void Sandbox::UpdateLevelSettings()
	{
		LP_PROFILE_FUNCTION();

		if (!m_LevelSettingsOpen)
		{
			return;
		}

		ImGui::Begin("Level Settings", &m_LevelSettingsOpen);

		ImGui::End();
	}

	void Sandbox::UpdateRenderingSettings()
	{
		LP_PROFILE_FUNCTION();

		if (!m_RenderingSettingsOpen)
		{
			return;
		}

		ImGui::Begin("Rendering Settings", &m_RenderingSettingsOpen);

		if (UI::TreeNodeFramed("Render graph", true))
		{
			static std::filesystem::path path = "";
			static std::filesystem::path lastPath = "";
			path = Renderer::GetRenderGraph() ? Renderer::GetRenderGraph()->Path : "";
			lastPath = path;

			if (UI::BeginProperties("renderProps"))
			{
				if (UI::Property("Graph", path))
				{
					auto graph = ResourceCache::GetAsset<RenderGraph>(path);
					if (!graph)
					{
						path = lastPath;
					}

					Renderer::SetRenderGraph(graph);
					Renderer::GetRenderGraph()->Start();
				}

				UI::EndProperties();
			}

			UI::TreeNodePop();
		}

		ImGui::End();
	}

	void Sandbox::UpdateToolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
		UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
		UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
		UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 4.f;
		Ref<Lamp::Texture2D> playIcon = m_IconPlay;
		if (m_SceneState == SceneState::Play)
		{
			playIcon = m_IconStop;
		}

		ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

		if (ImGui::ImageButton((ImTextureID)playIcon->GetID(), { size, size }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
		{
			if (m_SceneState == SceneState::Edit)
			{
				OnLevelPlay();
			}
			else if (m_SceneState == SceneState::Play)
			{
				OnLevelStop();
			}
		}

		Ref<Lamp::Texture2D> physicsIcon = m_PhysicsIcon.GetCurrentFrame();

		ImGui::SameLine();

		static uint32_t physicsId = physicsIcon->GetID();

		if (ImGui::ImageButtonAnimated((ImTextureID)physicsId, (ImTextureID)physicsIcon->GetID(), { size, size }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
		{
			if (m_SceneState == SceneState::Edit)
			{
				OnSimulationStart();
				m_PhysicsIcon.Play();
			}
			else if (m_SceneState == SceneState::Simulating)
			{
				OnSimulationStop();
				m_PhysicsIcon.Stop();
			}
		}
		ImGui::PopStyleVar(2);
		ImGui::End();
	}

	void Sandbox::UpdateStatistics()
	{
		ImGui::Begin("Statistics");

		ImGui::Text("Frame time: %f", Application::Get().GetFrameTime().GetFrameTime() * 1000);
		ImGui::Text("Frames per second: %f", Application::Get().GetFrameTime().GetFramesPerSecond());

		ImGui::End();
	}

	void Sandbox::CreateDockspace()
	{
		LP_PROFILE_FUNCTION();

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
		ImGui::Begin("MainDockspace", p, window_flags);
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

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowMinSize.x = 300.f;

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
					if (g_pEnv->pLevel->Path.empty())
					{
						SaveLevelAs();
					}
					else
					{
						g_pEnv->pAssetManager->SaveAsset(g_pEnv->pLevel);
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

			if (ImGui::BeginMenu("Editor"))
			{
				ImGui::MenuItem("Render Bounding Box", NULL, &g_pEnv->ShouldRenderBB);
				ImGui::MenuItem("Render Gizmos", NULL, &g_pEnv->ShouldRenderGizmos);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("Properties", NULL, &m_InspectiorOpen);
				ImGui::MenuItem("Create", NULL, &m_CreateToolOpen);
				ImGui::MenuItem("Log", NULL, &m_LogToolOpen);
				ImGui::MenuItem("Level Settings", NULL, &m_LevelSettingsOpen);
				ImGui::MenuItem("Asset Manager", NULL, &m_assetManager.GetIsOpen());

				for (auto pWindow : m_pWindows)
				{
					ImGui::MenuItem(pWindow->GetLabel().c_str(), NULL, &pWindow->GetIsOpen());
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Rendering"))
			{
				ImGui::MenuItem("Settings", NULL, &m_RenderingSettingsOpen);
				if (ImGui::MenuItem("Recompile shaders"))
				{
					Lamp::ShaderLibrary::RecompileShaders();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Play"))
			{
				if (ImGui::MenuItem("Play", "Ctrl + G"))
				{
					if (m_SceneState == SceneState::Edit)
					{
						OnLevelPlay();
					}
					else if (m_SceneState == SceneState::Play)
					{
						OnLevelStop();
					}
				}

				ImGui::MenuItem("Play physics", NULL, &Lamp::Application::Get().GetIsSimulating());

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();
	}

	bool Sandbox::OnMouseMoved(Lamp::MouseMovedEvent& e)
	{
		m_MouseHoverPos = glm::vec2(e.GetX(), e.GetY());

		return true;
	}
}