#include "lppch.h"
#include "Sandbox3D.h"

#include <imgui/misc/cpp/imgui_stdlib.h>

#include <Lamp/Objects/Entity/Base/Entity.h>
#include <Lamp/Objects/Entity/Base/ComponentRegistry.h>
#include <ImGuizmo/ImGuizmo.h>

#include <Lamp/Core/CoreLogger.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>

#include <Lamp/Math/Math.h>
#include "Windows/SandboxMeshImporter.h"
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <Lamp/Meshes/Materials/MaterialLibrary.h>

#include <Lamp/Core/Application.h>
#include <Lamp/GraphKey/GraphKeyGraph.h>
#include "Sandbox3D/Windows/GraphKey.h"

#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <Lamp/Rendering/RenderGraph/RenderGraph.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodeEnd.h>
#include <Lamp/Rendering/Vertices/FrameBuffer.h>

namespace Sandbox3D
{
	using namespace Lamp;

	void Sandbox3D::UpdatePerspective()
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
			if (Renderer3D::GetSettings().RenderGraph)
			{
				if (Renderer3D::GetSettings().RenderGraph->GetSpecification().endNode->framebuffer)
				{
 					textureID = Renderer3D::GetSettings().RenderGraph->GetSpecification().endNode->framebuffer->GetColorAttachmentID(0);
					m_SelectionBuffer = Renderer3D::GetSettings().RenderGraph->GetSpecification().endNode->framebuffer;
				}
			}
			ImGui::Image((void*)(uint64_t)textureID, ImVec2{ m_PerspectiveSize.x, m_PerspectiveSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* wPath = (const wchar_t*)pPayload->Data;
					std::filesystem::path path = std::filesystem::path("assets") / std::filesystem::path(wPath);

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

				if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("BRUSH_ITEM"))
				{
					const char* path = (const char*)pPayload->Data;
					m_pSelectedObject = Lamp::Brush::Create(path);
				}

				ImGui::EndDragDropTarget();
			}

			std::string frameInfo = "FrameTime: " + std::to_string(Lamp::Application::Get().GetFrameTime().GetFrameTime()) + ". FPS: " + std::to_string(Lamp::Application::Get().GetFrameTime().GetFramesPerSecond()) + ". Using VSync: " + std::to_string(Lamp::Application::Get().GetWindow().GetIsVSync());
			ImGui::SetCursorPos(ImVec2(20, 40));
			ImGui::Text(frameInfo.c_str());
		}

		//Guizmo
		static glm::mat4 transform = glm::mat4(1.f);
		static glm::mat4 lastTrans = glm::mat4(1.f);
		static bool beginMove = false;
		static bool hasStarted = false;
		static bool firstTime = true;

		if (m_pSelectedObject && m_SceneState != SceneState::Play)
		{
			transform = m_pSelectedObject->GetModelMatrix();

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
						auto brush = static_cast<Lamp::Brush*>(m_pSelectedObject);
						m_pSelectedObject = Lamp::Brush::Duplicate(brush, true);
					}
					else if (typeid(*m_pSelectedObject) == typeid(Lamp::Entity))
					{
						auto entity = static_cast<Lamp::Entity*>(m_pSelectedObject);
						m_pSelectedObject = Lamp::Entity::Duplicate(entity, true);
					}

					hasDuplicated = true;
				}
				else
				{
					glm::vec3 p, r, s;
					ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform), glm::value_ptr(p), glm::value_ptr(r), glm::value_ptr(s));

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

	void Sandbox3D::UpdateProperties()
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

					if (m_pSelectedObject)
					{
						m_pSelectedObject->SetIsSelected(false);
					}

					m_pSelectedObject = Lamp::Entity::Get(pixelData);
					if (!m_pSelectedObject)
					{
						m_pSelectedObject = Lamp::Brush::Get(pixelData);
					}

					if (m_pSelectedObject)
					{
						m_pSelectedObject->SetIsSelected(true);
					}
				}

				m_SelectionBuffer->Unbind();
				////////////////////////
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


					glm::vec3 rot = pEnt->GetRotation();
					float r[3] = { rot.x, rot.y, rot.z };

					ImGui::InputFloat3("Rotation", r);
					pEnt->SetRotation(glm::make_vec3(r));

					glm::vec3 scale = pEnt->GetScale();
					float s[3] = { scale.x, scale.y, scale.z };

					ImGui::InputFloat3("Scale", s);
					pEnt->SetScale(glm::make_vec3(s));
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

				if (ImGui::CollapsingHeader("Materials"))
				{
					int i = 0;
					for (auto& mat : pBrush->GetModel()->GetMaterials())
					{
						static std::string lastInput = "";
						std::string input = mat.second->GetName();
						
						ImGui::InputText(std::string("###input" + std::to_string(i)).c_str(), &input);
						if (input != lastInput)
						{
							lastInput = input;
						}

						i++;
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

	void Sandbox3D::UpdateLogTool()
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
		LP_PROFILE_FUNCTION();

		bool removeComp = false;

		if (ImGui::CollapsingHeader(ptr->GetName().c_str()))
		{
			std::string id = "Remove###Remove" + ptr->GetName();
			if (ImGui::Button(id.c_str()))
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

					ImGui::DragInt(pProp.Name.c_str(), &v);
					if (v != *p)
					{
						*p = v;

						Lamp::EntityPropertyChangedEvent e;
						ptr->GetEntity()->OnEvent(e);
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
						ptr->GetEntity()->OnEvent(e);
					}

					break;
				}

				case Lamp::PropertyType::Float:
				{
					float* p = static_cast<float*>(pProp.Value);
					float v = *p;

					ImGui::DragFloat(pProp.Name.c_str(), &v);
					if (v != *p)
					{
						*p = v;

						Lamp::EntityPropertyChangedEvent e;
						ptr->GetEntity()->OnEvent(e);
					}
					break;
				}

				case Lamp::PropertyType::Float2:
				{
					glm::vec2* p = static_cast<glm::vec2*>(pProp.Value);
					glm::vec2 v = *p;

					ImGui::DragFloat2(pProp.Name.c_str(), glm::value_ptr(v));
					if (v != *p)
					{
						*p = v;

						Lamp::EntityPropertyChangedEvent e;
						ptr->GetEntity()->OnEvent(e);
					}
					break;
				}

				case Lamp::PropertyType::Float3:
				{
					glm::vec3* p = static_cast<glm::vec3*>(pProp.Value);
					glm::vec3 v = *p;

					ImGui::DragFloat3(pProp.Name.c_str(), glm::value_ptr(v));
					if (v != *p)
					{
						*p = v;

						Lamp::EntityPropertyChangedEvent e;
						ptr->GetEntity()->OnEvent(e);
					}
					break;
				}

				case Lamp::PropertyType::Float4:
				{
					glm::vec4* p = static_cast<glm::vec4*>(pProp.Value);
					glm::vec4 v = *p;

					ImGui::DragFloat4(pProp.Name.c_str(), glm::value_ptr(*p));
					if (v != *p)
					{
						*p = v;

						Lamp::EntityPropertyChangedEvent e;
						ptr->GetEntity()->OnEvent(e);
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
						ptr->GetEntity()->OnEvent(e);
					}
					break;
				}

				case Lamp::PropertyType::Path:
				{
					std::string* s = static_cast<std::string*>(pProp.Value);
					std::string v = *s;

					ImGui::InputText(pProp.Name.c_str(), &v);
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
						{
							const wchar_t* path = (const wchar_t*)pPayload->Data;
							std::filesystem::path p = std::filesystem::path("assets") / path;
							v = p.string();
						}

						ImGui::EndDragDropTarget();
					}

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
						ptr->GetEntity()->OnEvent(e);
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
						ptr->GetEntity()->OnEvent(e);
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
						ptr->GetEntity()->OnEvent(e);
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
		LP_PROFILE_FUNCTION();

		if (!m_LevelSettingsOpen)
		{
			return;
		}

		ImGui::Begin("Level Settings", &m_LevelSettingsOpen);

		ImGui::End();
	}

	void Sandbox3D::UpdateRenderingSettings()
	{
		LP_PROFILE_FUNCTION();

		if (!m_RenderingSettingsOpen)
		{
			return;
		}

		ImGui::Begin("Rendering Settings", &m_RenderingSettingsOpen);

		ImGui::Text("RenderGraph");

		std::string graphPath = Renderer3D::GetSettings().RenderGraph ? Renderer3D::GetSettings().RenderGraph->Path.string() : "";
		if (ImGui::InputText("##graph", &graphPath))
		{
			Lamp::Renderer3D::GetSettings().RenderGraph = ResourceCache::GetAsset<RenderGraph>(graphPath);
		}

		ImGui::SameLine();
		if (ImGui::Button("Open"))
		{
			graphPath = FileDialogs::OpenFile("RenderGraph (*.rendergraph)\0*.rendergraph\0");
			Lamp::Renderer3D::GetSettings().RenderGraph = ResourceCache::GetAsset<RenderGraph>(graphPath);

			Lamp::Renderer3D::GetSettings().RenderGraph->Start();
		}

		ImGui::SameLine();
		if (ImGui::Button("Reload"))
		{
			if (Renderer3D::GetSettings().RenderGraph)
			{
				Renderer3D::GetSettings().RenderGraph = ResourceCache::ReloadAsset<RenderGraph>(std::dynamic_pointer_cast<Asset>(Renderer3D::GetSettings().RenderGraph));
				Renderer3D::GetSettings().RenderGraph->Start();
			}
		}

		ImGui::Separator();

		ImGui::Text("HDR");

		static std::string path = "";
		ImGui::DragFloat("HDR Exposure", &Lamp::Renderer3D::GetSettings().HDRExposure);
		ImGui::InputText("HDR environment", &path);
		ImGui::SameLine();
		if (ImGui::Button("Open..."))
		{
			path = Lamp::FileDialogs::OpenFile("All (*.*)\0*.*\0");
			if (!path.empty())
			{
				Lamp::Renderer3D::SetEnvironment(path);
			}

			ImGui::DragFloat("HDR Exposure", &g_pEnv->HDRExposure);
		}

		ImGui::Separator();

		ImGui::Text("SSAO");

		int preKernelSize = Lamp::Renderer3D::GetSettings().SSAOKernelSize;
		ImGui::SliderInt("Kernel Size", &Lamp::Renderer3D::GetSettings().SSAOKernelSize, 16, Lamp::Renderer3D::GetSettings().SSAOMaxKernelSize, "%d");
		ImGui::SliderFloat("Radius", &Lamp::Renderer3D::GetSettings().SSAORadius, 0.f, 10.f, "%.3f");
		ImGui::SliderFloat("Bias", &Lamp::Renderer3D::GetSettings().SSAOBias, 0.f, 1.f, "%.3f");

		ImGui::Separator();
		ImGui::Text("General");

		ImGui::SliderFloat("Gamma", &Lamp::Renderer3D::GetSettings().Gamma, 0.f, 10.f, "%.3f");

		ImGui::End();
	}

	void Sandbox3D::UpdateRenderPassView()
	{
		if (!m_RenderPassViewOpen)
		{
			return;
		}

		ImGui::Begin("Render pass view", &m_RenderPassViewOpen);

		auto& passes = Lamp::RenderPassManager::Get()->GetRenderPasses();
		for (auto& pass : passes)
		{
			if (ImGui::CollapsingHeader(pass->GetSpecification().Name.c_str()))
			{
			}
		}

		ImGui::End();
	}

	void Sandbox3D::UpdateShaderView()
	{
		if (!m_ShaderViewOpen)
		{
			return;
		}

		ImGui::Begin("Shader View", &m_ShaderViewOpen);

		static auto& shaders = Lamp::ShaderLibrary::GetShaders();
		for (auto& shader : shaders)
		{
			if (ImGui::CollapsingHeader(shader->GetName().c_str()))
			{
				ImGui::Text("Path: %s", shader->GetPath().c_str());
				ImGui::Separator();

				ImGui::Text("Uniforms");
				for (auto& uniform : shader->GetSpecifications().Uniforms)
				{
					ImGui::Selectable(uniform.first.c_str());
				}
			}
		}

		ImGui::End();
	}

	void Sandbox3D::UpdateToolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.305f, 0.31f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.505f, 0.51f, 0.5f));

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
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void Sandbox3D::CreateDockspace()
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
				ImGui::MenuItem("Render pass view", NULL, &m_RenderPassViewOpen);
				ImGui::MenuItem("Shader View", NULL, &m_ShaderViewOpen);
				if (ImGui::MenuItem("Recompile shaders"))
				{
					Lamp::ShaderLibrary::RecompileShaders();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Buffers"))
			{
				for (auto& window : m_BufferWindows)
				{
					ImGui::MenuItem(window.GetLabel().c_str(), NULL, &window.GetIsOpen());
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

	bool Sandbox3D::OnMouseMoved(Lamp::MouseMovedEvent& e)
	{
		m_MouseHoverPos = glm::vec2(e.GetX(), e.GetY());

		return true;
	}
}