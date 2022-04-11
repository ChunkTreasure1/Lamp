#include "lppch.h"
#include "SandboxLayer.h"

#include "Sandbox/Windows/GraphKey.h"
#include "Sandbox/Windows/MeshImporterPanel.h"
#include "Sandbox/Actions/EditorCommands.h"

#include <Lamp/Objects/Entity/Entity.h>
#include <Lamp/Objects/Entity/ComponentRegistry.h>

#include <Lamp/Core/CoreLogger.h>
#include <Lamp/Core/Application.h>

#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Utility/UIUtility.h>

#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Rendering/Buffers/FrameBuffer.h>
#include <Lamp/Rendering/Renderer.h>

#include <Lamp/Math/Math.h>
#include <Lamp/Mesh/Materials/MaterialLibrary.h>
#include <Lamp/Mesh/MeshInstance.h>

#include <Lamp/GraphKey/GraphKeyGraph.h>
#include <Lamp/AssetSystem/ResourceCache.h>

#include <ImGuizmo/ImGuizmo.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

namespace Sandbox
{
	using namespace Lamp;

	namespace Utility
	{
		static std::string RemoveTrailingZeroes(const std::string& string)
		{
			std::string newStr = string;
			newStr.erase(newStr.find_last_not_of('0') + 1, std::string::npos);
			
			if (!newStr.empty() && newStr.back() == '.')
			{
				newStr.pop_back();
			}

			return newStr;
		}
	}

	void SandboxLayer::UpdatePerspective()
	{
		LP_PROFILE_FUNCTION();

		if (!m_perspectiveOpen)
		{
			return;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{ 0.f, 0.f });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });


		ImGui::Begin("Perspective", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			m_perspectiveHover = ImGui::IsWindowHovered();
			m_sandboxController->GetCameraController()->SetControlsEnabled(m_perspectiveHover);
			m_perspectiveFocused = ImGui::IsWindowFocused();

			m_sandboxController->GetCameraController()->SetActive(m_perspectiveHover);

			const float toolBarSize = 22.f;
			const float toolBarYPadding = 5.f;
			const float toolBarXPadding = 5.f;

			//Viewport bounds
			auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
			auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
			auto viewportOffset = ImGui::GetWindowPos();

			m_perspectiveBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y - (toolBarSize + toolBarYPadding) };
			m_perspectiveBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y - (toolBarSize + toolBarYPadding) };

			ImVec2 perspectivePanelSize = ImGui::GetContentRegionAvail();
			perspectivePanelSize.y -= (toolBarSize + toolBarYPadding);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.f, 0.f });
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.f, 0.f });

			/////Perspective toolbar/////
			{
				UI::ScopedColor childBg(ImGuiCol_ChildBg, { 0.258f, 0.258f, 0.258f, 1.000f });
				ImGui::BeginChild("toolbarChild", { ImGui::GetContentRegionAvail().x, toolBarSize + toolBarYPadding }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
				UI::ShiftCursor(toolBarXPadding, toolBarYPadding / 2.f);

				UpdatePerspectiveToolbar(toolBarSize, toolBarXPadding);
				ImGui::EndChild();
			}
			/////////////////////////////

			if (m_perspectiveSize != *((glm::vec2*)&perspectivePanelSize))
			{
				m_perspectiveSize = { perspectivePanelSize.x, perspectivePanelSize.y };

				Lamp::EditorViewportSizeChangedEvent e((uint32_t)perspectivePanelSize.x, (uint32_t)perspectivePanelSize.y);
				OnEvent(e);

				if (LevelManager::IsLevelLoaded() && perspectivePanelSize.x > 0.f && perspectivePanelSize.y > 0.f)
				{
					LevelManager::GetActive()->OnEvent(e);
				}
			}

			if (LevelManager::IsLevelLoaded())
			{
				ImGui::Image(UI::GetTextureID(LevelManager::GetActive()->GetFinalRenderedImage()), ImVec2{ m_perspectiveSize.x, m_perspectiveSize.y }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });
			}

			ImGui::PopStyleVar(2);

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
					m_pSelectedObject = Lamp::Brush::Create(path);
				}
			}
		}

		//Guizmo
		static glm::mat4 transform = glm::mat4(1.f);
		static glm::mat4 startTransform = glm::mat4(1.f);

		if (m_pSelectedObject && m_sceneState != SceneState::Play)
		{
			transform = m_pSelectedObject->GetTransform();

			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_perspectiveBounds[0].x, m_perspectiveBounds[0].y, m_perspectiveBounds[1].x - m_perspectiveBounds[0].x, m_perspectiveBounds[1].y - m_perspectiveBounds[0].y);

			bool snap = Lamp::Input::IsKeyPressed(LP_KEY_LEFT_CONTROL);
			bool duplicate = Lamp::Input::IsKeyPressed(LP_KEY_LEFT_SHIFT);

			float snapValue = 0.5f;
			if (m_imGuizmoOperation == ImGuizmo::ROTATE)
			{
				snap = m_snapRotation && snap ? false : m_snapRotation && !snap ? true : snap;
				snapValue = m_rotateSnapValue;
			}
			else if (m_imGuizmoOperation == ImGuizmo::SCALE)
			{
				snap = m_snapScale && snap ? false : m_snapScale && !snap ? true : snap;
				snapValue = m_scaleSnapValue;
			}
			else if (m_imGuizmoOperation == ImGuizmo::TRANSLATE)
			{
				snap = m_snapToGrid && snap ? false : m_snapToGrid && !snap ? true : snap;
				snapValue = m_gridSnapValue;
			}

			float snapValues[3] = { snapValue, snapValue, snapValue };
			
			ImGuizmo::MODE gizmoMode = m_worldSpace ? ImGuizmo::WORLD : ImGuizmo::LOCAL;

			ImGuizmo::Manipulate(
				glm::value_ptr(m_sandboxController->GetCameraController()->GetCamera()->GetViewMatrix()),
				glm::value_ptr(m_sandboxController->GetCameraController()->GetCamera()->GetProjectionMatrix()),
				m_imGuizmoOperation, gizmoMode, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			static bool hasDuplicated = false;

			static bool isUsing = false;
			static bool lastUsed = false;

			lastUsed = isUsing;
			isUsing = ImGuizmo::IsUsing();

			if (isUsing && !lastUsed && transform != m_pSelectedObject->GetTransform())
			{
				startTransform = m_pSelectedObject->GetTransform();
			}

			if (!isUsing && lastUsed && transform != m_pSelectedObject->GetTransform())
			{
				Ref<TransformCommand> transformCmd = CreateRef<TransformCommand>(m_pSelectedObject, startTransform);
				m_perspectiveCommands.Push(transformCmd);
			}

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

						Ref<DuplicateCommand> cmd = CreateRef<DuplicateCommand>(brush, m_pSelectedObject, &m_pSelectedObject);
						m_perspectiveCommands.Push(cmd);
					}
					else if (typeid(*m_pSelectedObject) == typeid(Lamp::Entity))
					{
						m_pSelectedObject->SetIsSelected(false);

						auto entity = static_cast<Lamp::Entity*>(m_pSelectedObject);
						m_pSelectedObject = Lamp::Entity::Duplicate(entity, true);

						m_pSelectedObject->SetIsSelected(true);

						Ref<DuplicateCommand> cmd = CreateRef<DuplicateCommand>(entity, m_pSelectedObject, &m_pSelectedObject);
						m_perspectiveCommands.Push(cmd);
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

	void SandboxLayer::UpdateProperties()
	{
		LP_PROFILE_FUNCTION();

		if (!m_inspectorOpen)
		{
			return;
		}

		ImGui::Begin("Properties", &m_inspectorOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDecoration);
		{
			UI::ScopedColor buttonColor(ImGuiCol_Button, { 0.313f, 0.313f, 0.313f, 1.f });
			UI::ScopedStyleFloat buttonRounding(ImGuiStyleVar_FrameRounding, 2.f);

			ImGuiIO& io = ImGui::GetIO();
			glm::vec2 mousePos = glm::vec2(io.MouseClickedPos->x, io.MouseClickedPos->y);
			//glm::vec2 windowPos;
			bool perspHover = false;

			//if (m_MousePressed && perspHover && !ImGuizmo::IsOver() && m_SceneState != SceneState::Play)
			//{
			//	mousePos -= windowPos;

			//	/////Mouse picking/////

			//	auto [mx, my] = ImGui::GetMousePos();
			//	mx -= m_PerspectiveBounds[0].x;
			//	my -= m_PerspectiveBounds[0].y;

			//	glm::vec2 perspectiveSize = m_PerspectiveBounds[1] - m_PerspectiveBounds[0];
			//	my = perspectiveSize.y - my;

			//	int mouseX = (int)mx;
			//	int mouseY = (int)my;

			//	if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)perspectiveSize.x && mouseY < (int)perspectiveSize.y)
			//	{
		//			int pixelData = m_SelectionBuffer->ReadPixel(1, mouseX, mouseY);

			//		Object* newSelected = Lamp::Entity::Get(pixelData);
			//		if (!newSelected)
			//		{
			//			newSelected = Lamp::Brush::Get(pixelData);
			//		}

			//		if (newSelected && !newSelected->GetIsFrozen())
			//		{
			//			if (m_pSelectedObject)
			//			{
			//				m_pSelectedObject->SetIsSelected(false);
			//			}

			//			newSelected->SetIsSelected(true);
			//			m_pSelectedObject = newSelected;
			//		}
			//		else
			//		{
			//			if (m_pSelectedObject)
			//			{
			//				m_pSelectedObject->SetIsSelected(false);
			//			}
			//			m_pSelectedObject = nullptr;
			//		}
			//	}

			//	m_SelectionBuffer->Unbind();
			//	////////////////////////
			//}

			if (auto pEnt = dynamic_cast<Lamp::Entity*>(m_pSelectedObject))
			{
				ImGui::Text("Entity");

				std::string name = pEnt->GetName();
				UI::InputText("Name", name);
				pEnt->SetName(name);

				if (UI::TreeNodeFramed("Transform", true))
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

						if (auto vs = (GraphKey*)(m_windows[1]))
						{
							vs->SetIsOpen(true);
							vs->SetCurrentlyOpenGraph(pEnt->GetGraphKeyGraph(), pEnt->GetID());
						}
					}
					else
					{
						if (auto vs = (GraphKey*)(m_windows[1]))
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

				if (UI::TreeNodeFramed("Transform", true))
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
					for (auto& mat : pBrush->GetMesh()->GetSharedMesh()->GetMaterials())
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

			if (auto pEnt = dynamic_cast<Lamp::Entity*>(m_pSelectedObject))
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

	void SandboxLayer::UpdateAddComponent()
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

	void SandboxLayer::UpdateMainToolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
		UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
		UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
		UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });
		
		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		
		float size = ImGui::GetWindowHeight() - 4.f;
		
		if (UI::ImageButton("##save", UI::GetTextureID(m_iconSave), { size, size }))
		{

		}
		
		ImGui::SameLine();

		if (UI::ImageButton("##load", UI::GetTextureID(m_iconLoad), { size, size }))
		{

		}

		ImGui::End();
		ImGui::PopStyleVar(2);
	}

	void SandboxLayer::UpdateLogTool()
	{
		LP_PROFILE_FUNCTION();

		static bool autoScroll = true;

		if (!m_logToolOpen)
		{
			return;
		}

		ImGui::Begin("Log", &m_logToolOpen);

		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &autoScroll);
			ImGui::EndPopup();
		}

		if (ImGui::Button("Clear"))
		{
			//Lamp::Log::GetCoreLogger()->GetMessages().clear();
		}
		ImGui::SameLine();
		if (ImGui::Button("Options"))
		{
			ImGui::OpenPopup("Options");
		}

		ImGui::Separator();

		ImGui::BeginChild("text");

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		//for (const auto& msg : Lamp::Log::GetCoreLogger()->GetMessages())
		//{
		//	if (msg.Level == Lamp::LogLevel::Trace)
		//	{
		//		ImGui::TextColored(ImVec4(212.f / 255.f, 212.f / 255.f, 212.f / 255.f, 1.f), msg.Msg.c_str());
		//	}
		//	else if (msg.Level == Lamp::LogLevel::Info)
		//	{
		//		ImGui::Text(msg.Msg.c_str());
		//	}
		//	else if (msg.Level == Lamp::LogLevel::Warn)
		//	{
		//		ImGui::TextColored(ImVec4(255.f / 255.f, 235.f / 255.f, 54.f / 255.f, 1.f), msg.Msg.c_str());
		//	}
		//	else if (msg.Level == Lamp::LogLevel::Error)
		//	{
		//		ImGui::TextColored(ImVec4(255.f / 255.f, 0, 0, 1.f), msg.Msg.c_str());
		//	}
		//	else if (msg.Level == Lamp::LogLevel::Critical)
		//	{
		//		ImGui::TextColored(ImVec4(255.f / 255.f, 0, 0, 1.f), msg.Msg.c_str());
		//	}
		//}
		ImGui::PopStyleVar();

		if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.f);
		}
		ImGui::EndChild();
		ImGui::End();
	}

	bool SandboxLayer::DrawComponent(Lamp::EntityComponent* ptr)
	{
		LP_PROFILE_FUNCTION();

		bool removeComp = false;
		bool open = UI::TreeNodeFramed(ptr->GetName(), false, true, 0.f, { 0.f, 2.f });

		float buttonSize = 21.f + GImGui->Style.FramePadding.y;
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

			bool propertyChanged = false;

			for (auto& prop : ptr->GetComponentProperties().GetProperties())
			{
				switch (prop.propertyType)
				{
					case PropertyType::Int: propertyChanged = UI::Property(prop.name, *static_cast<int*>(prop.value)); break;
					case PropertyType::Bool: propertyChanged = UI::Property(prop.name, *static_cast<bool*>(prop.value)); break;
					case PropertyType::Float: propertyChanged = UI::Property(prop.name, *static_cast<float*>(prop.value)); break;
					case PropertyType::Float2: propertyChanged = UI::Property(prop.name, *static_cast<glm::vec2*>(prop.value)); break;
					case PropertyType::Float3: propertyChanged = UI::Property(prop.name, *static_cast<glm::vec3*>(prop.value)); break;
					case PropertyType::Float4: propertyChanged = UI::Property(prop.name, *static_cast<glm::vec4*>(prop.value)); break;
					case PropertyType::Color3: propertyChanged = UI::PropertyColor(prop.name, *static_cast<glm::vec3*>(prop.value)); break;
					case PropertyType::Color4: propertyChanged = UI::PropertyColor(prop.name, *static_cast<glm::vec4*>(prop.value)); break;
					case PropertyType::String: propertyChanged = UI::Property(prop.name, *static_cast<std::string*>(prop.value)); break;
					case PropertyType::Path:
					{
						std::filesystem::path path = std::filesystem::path(*static_cast<std::string*>(prop.value));

						propertyChanged = UI::Property(prop.name, path);

						*static_cast<std::string*>(prop.value) = path.string();
						break;
					}
				}
			}

			if (propertyChanged)
			{
				Lamp::ObjectPropertyChangedEvent e;
				ptr->GetEntity()->OnEvent(e);
			}

			UI::EndProperties();
			UI::TreeNodePop();
		}

		return removeComp;
	}

	void SandboxLayer::UpdateLevelSettings()
	{
		LP_PROFILE_FUNCTION();

		if (!m_levelSettingsOpen)
		{
			return;
		}

		ImGui::Begin("Level Settings", &m_levelSettingsOpen);

		ImGui::TextUnformatted("Terrain");
		ImGui::Separator();

		if (UI::BeginProperties("Terrain"))
		{
			UI::Property("Displacement factor", const_cast<float&>(Renderer::Get().GetStorage().terrainData.displacementFactor));
			UI::Property("Tessellation factor", const_cast<float&>(Renderer::Get().GetStorage().terrainData.tessellationFactor));
			UI::Property("Tessellated edge size", const_cast<float&>(Renderer::Get().GetStorage().terrainData.tessellatedEdgeSize));

			UI::EndProperties(false);
			UI::PopId();
		}

		ImGui::Separator();

		ImGui::End();
	}

	void SandboxLayer::UpdateRenderingSettings()
	{
		LP_PROFILE_FUNCTION();

		if (!m_RenderingSettingsOpen)
		{
			return;
		}

		auto sceneData = const_cast<RendererStorage&>(Renderer::Get().GetStorage());

		ImGui::Begin("Rendering Settings", &m_RenderingSettingsOpen);

		if (UI::TreeNodeFramed("General", true))
		{
			ImGui::Text("HDR");
			ImGui::Separator();
			if (UI::BeginProperties("generalProps"))
			{
				//UI::Property("Exposure", sceneData.hdrExposure); // move to level environment
				//UI::Property("Gamma", sceneData->gamma); 

				UI::EndProperties();
			}

			UI::TreeNodePop();
		}

		if (UI::TreeNodeFramed("Render graph", true))
		{
			static std::filesystem::path path = "";
			static std::filesystem::path lastPath = "";
			lastPath = path;

			if (UI::BeginProperties("renderProps"))
			{
				if (UI::Property("Graph", path))
				{

				}

				UI::EndProperties();
			}

			UI::TreeNodePop();
		}

		ImGui::End();
	}

	void SandboxLayer::UpdatePerspectiveToolbar(float toolBarHeight, float toolBarXPadding)
	{
		UI::ScopedColor button(ImGuiCol_Button, { 0.f, 0.f, 0.f, 0.f });
		UI::ScopedColor hovered(ImGuiCol_ButtonHovered, { 0.3f, 0.305f, 0.31f, 0.5f });
		UI::ScopedColor active(ImGuiCol_ButtonActive, { 0.5f, 0.505f, 0.51f, 0.5f });
		
		UI::ScopedStyleFloat2 itemSpacing(ImGuiStyleVar_ItemSpacing, { 2.f, 0.f });

		Ref<Lamp::Texture2D> playIcon = m_iconPlay;
		if (m_sceneState == SceneState::Play)
		{
			playIcon = m_iconStop;
		}

		if (UI::ImageButton("##play", UI::GetTextureID(playIcon), { toolBarHeight, toolBarHeight }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
		{
			if (m_sceneState == SceneState::Edit)
			{
				OnLevelPlay();
			}
			else if (m_sceneState == SceneState::Play)
			{
				OnLevelStop();
			}
		}

		Ref<Lamp::Texture2D> physicsIcon = m_physicsIcon.GetCurrentFrame();

		ImGui::SameLine();

		if (UI::ImageButton("##physicsPlay", UI::GetTextureID(physicsIcon), { toolBarHeight, toolBarHeight }, { 0.f, 0.f }, { 1.f, 1.f }, 0))
		{
			if (m_sceneState == SceneState::Edit)
			{
				OnSimulationStart();
				m_physicsIcon.Play();
			}
			else if (m_sceneState == SceneState::Simulating)
			{
				OnSimulationStop();
				m_physicsIcon.Stop();
			}
		}
		const uint32_t rightButtonCount = 5;
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - toolBarXPadding - (rightButtonCount * toolBarHeight));
		
		Ref<Texture2D> localWorldIcon;
		if (m_worldSpace)
		{
			localWorldIcon = m_iconWorldSpace;
		}
		else
		{
			localWorldIcon = m_iconLocalSpace;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.f);
		if (UI::ImageButton("##localWorld", UI::GetTextureID(localWorldIcon), { toolBarHeight, toolBarHeight }, { 0.f, 1.f }, { 1.f, 0.f }))
		{
			m_worldSpace = !m_worldSpace;
		}

		ImGui::SameLine();

		if (UI::ImageButtonState("##snapToGrid", m_snapToGrid, UI::GetTextureID(m_iconSnapToGrid), { toolBarHeight, toolBarHeight }))
		{
			m_snapToGrid = !m_snapToGrid;
		}

		ImGui::SetNextWindowSize({ 100.f, m_snapToGridValues.size() * 20.f });
		if (ImGui::BeginPopupContextItem("##gridSnapValues", ImGuiPopupFlags_MouseButtonRight))
		{
			for (uint32_t i = 0; i < m_snapToGridValues.size(); i++)
			{
				std::string valueStr = Utility::RemoveTrailingZeroes(std::to_string(m_snapToGridValues[i]));
				std::string	id = valueStr + "##gridSnapValue" + std::to_string(i);

				if (ImGui::Selectable(id.c_str()))
				{
					m_gridSnapValue = m_snapToGridValues[i];
				}
			}

			ImGui::EndPopup();
		}
		ImGui::SameLine();

		if (UI::ImageButtonState("##snapRotation", m_snapRotation, UI::GetTextureID(m_iconSnapRotation), { toolBarHeight, toolBarHeight }))
		{
			m_snapRotation = !m_snapRotation;
		}
		if (ImGui::BeginPopupContextItem("##rotateSnapValues", ImGuiPopupFlags_MouseButtonRight))
		{
			for (uint32_t i = 0; i < m_snapRotationValues.size(); i++)
			{
				std::string valueStr = Utility::RemoveTrailingZeroes(std::to_string(m_snapRotationValues[i]));
				std::string	id = valueStr + "##rotationSnapValue" + std::to_string(i);

				if (ImGui::Selectable(id.c_str()))
				{
					m_rotateSnapValue = m_snapRotationValues[i];
				}
			}

			ImGui::EndPopup();
		}
		
		ImGui::SameLine();

		if (UI::ImageButtonState("##snapScale", m_snapScale, UI::GetTextureID(m_iconSnapScale), { toolBarHeight, toolBarHeight }))
		{
			m_snapScale = !m_snapScale;
		}
		if (ImGui::BeginPopupContextItem("##scaleSnapValues", ImGuiPopupFlags_MouseButtonRight))
		{
			for (uint32_t i = 0; i < m_snapScaleValues.size(); i++)
			{
				std::string valueStr = Utility::RemoveTrailingZeroes(std::to_string(m_snapScaleValues[i]));
				std::string	id = valueStr + "##scaleSnapValue" + std::to_string(i);

				if (ImGui::Selectable(id.c_str()))
				{
					m_scaleSnapValue = m_snapScaleValues[i];
				}
			}

			ImGui::EndPopup();
		}

		ImGui::SameLine();

		if (UI::ImageButtonState("##showGizmos", m_showGizmos, UI::GetTextureID(m_iconShowGizmos), { toolBarHeight, toolBarHeight }))
		{
			m_showGizmos = !m_showGizmos;
		}

		ImGui::PopStyleVar();
	}
	

	void SandboxLayer::UpdateStatistics()
	{
		ImGui::Begin("Statistics");

		if (UI::TreeNodeFramed("Application"))
		{
			ImGui::Text("Application time: %f ms", Application::Get().GetMainFrameTime().GetFrameTime() * 1000);
			ImGui::Text("Frames per second: %f FPS", Application::Get().GetMainFrameTime().GetFramesPerSecond());

			UI::TreeNodePop();
		}

		if (UI::TreeNodeFramed("UI"))
		{
			ImGui::Text("Total time: %f ms", m_uiTotalTime * 1000.f);
			ImGui::Text("Asset Manager: %f ms", m_assetManagerTime * 1000.f);
			ImGui::Text("Create panel: %f ms", m_createPanelTime * 1000.f);

			for (const auto window : m_windows)
			{
				std::string text = window->GetName() + ": %f ms";
				ImGui::Text(text.c_str(), window->GetDeltaTime() * 1000.f);
			}

			UI::TreeNodePop();
		}

		if (UI::TreeNodeFramed("GPU"))
		{
			const auto& stats = Renderer::Get().GetStatistics();

			ImGui::Text("Total draw calls: %d", stats.totalDrawCalls.load());
			ImGui::Text("Culled draw calls: %d", stats.culledDrawCalls.load());

			ImGui::Text("Total memory: %d MBs", UI::BytesToMBs(stats.memoryStatistics.totalGPUMemory));
			ImGui::Text("Allocated memory: %d MBs", UI::BytesToMBs(stats.memoryStatistics.allocatedMemory));
			ImGui::Text("Free memory: %d MBs", UI::BytesToMBs(stats.memoryStatistics.freeMemory));

			ImGui::Text("Total allocated memory: %d MBs", UI::BytesToMBs(stats.memoryStatistics.totalAllocatedMemory));
			ImGui::Text("Total freed memory: %d MBs", UI::BytesToMBs(stats.memoryStatistics.totalFreedMemory));

			UI::TreeNodePop();
		}

		ImGui::End();
	}

	void SandboxLayer::CreateDockspace()
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
					if (!LevelManager::IsLevelLoaded())
					{
						LP_WARN("Trying to save when no level was loaded!");
					}
					else
					{
						if (!std::filesystem::exists(LevelManager::GetActive()->Path))
						{
							SaveLevelAs();
						}
						else
						{
							g_pEnv->pAssetManager->SaveAsset(LevelManager::GetActive());
						}
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
				ImGui::MenuItem("Render Bounding Box", nullptr, &g_pEnv->shouldRenderBB);
				ImGui::MenuItem("Render Gizmos", nullptr, &g_pEnv->shouldRenderGizmos);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("Properties", nullptr, &m_inspectorOpen);
				ImGui::MenuItem("Log", nullptr, &m_logToolOpen);
				ImGui::MenuItem("Level Settings", nullptr, &m_levelSettingsOpen);

				for (auto pWindow : m_windows)
				{
					ImGui::MenuItem(pWindow->GetLabel().c_str(), nullptr, &pWindow->GetIsOpen());
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Rendering"))
			{
				ImGui::MenuItem("Settings", nullptr, &m_RenderingSettingsOpen);
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
					if (m_sceneState == SceneState::Edit)
					{
						OnLevelPlay();
					}
					else if (m_sceneState == SceneState::Play)
					{
						OnLevelStop();
					}
				}

				if (ImGui::MenuItem("Play physics", nullptr))
				{

				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();
	}

	bool SandboxLayer::OnMouseMoved(Lamp::MouseMovedEvent& e)
	{
		return true;
	}
}