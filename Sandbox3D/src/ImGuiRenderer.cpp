#include "lppch.h"
#include "Sandbox3D.h"

#include <Lamp/Level/LevelSystem.h>
#include <ImGuizmo/ImGuizmo/ImGuizmo.h>
#include <Lamp/Rendering/Renderer3D.h>
#include <Lamp/Meshes/GeometrySystem.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <Lamp/Objects/ObjectLayer.h>

namespace Sandbox3D
{
	void Sandbox3D::UpdatePerspective()
	{
		ImGui::Begin("Perspective");
		{
			m_PerspectiveHover = ImGui::IsWindowHovered();
			m_PerspectiveCamera.SetCameraControlsEnabled(m_PerspectiveHover);

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Aspect ratio"))
				{

				}
				ImGui::EndMenuBar();
			}

			ImGui::GetWindowDrawList()->AddImage((void*)(uint64_t)Lamp::Renderer3D::GetFrameBuffer()->GetTexture(),
				ImVec2(ImGui::GetCursorScreenPos()),
				ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetWindowSize().x, ImGui::GetCursorScreenPos().y + ImGui::GetWindowSize().y),
				ImVec2(0, 1),
				ImVec2(1, 0));

			m_PerspectiveCamera.UpdatePerspective(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
			Lamp::Renderer3D::GetFrameBuffer()->Update((const uint32_t)ImGui::GetWindowSize().x, (const uint32_t)ImGui::GetWindowSize().y);
			m_PerspectiveSize = glm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

			std::string frameInfo = "FrameTime: " + std::to_string(Lamp::Application::Get().GetFrameTime().GetFrameTime()) + ". FPS: " + std::to_string(Lamp::Application::Get().GetFrameTime().GetFramesPerSecond()) + ". Using VSync: " + std::to_string(Lamp::Application::Get().GetWindow().GetIsVSync());

			ImGui::Text(frameInfo.c_str());
		}
		ImGui::End();

		static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
		static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

		ImGuizmo::SetRect(0, 0, m_PerspectiveSize.x, m_PerspectiveSize.y);

		if (m_pSelectedBrush)
		{
			float* pMat = (float*)glm::value_ptr(m_pSelectedBrush->GetModelMatrix());
			ImGuizmo::Manipulate((const float*)glm::value_ptr(m_PerspectiveCamera.GetCamera().GetViewMatrix()), (const float*)glm::value_ptr(m_PerspectiveCamera.GetCamera().GetProjectionMatrix()), mCurrentGizmoOperation, mCurrentGizmoMode, pMat);
			m_pSelectedBrush->SetModelMatrix(glm::make_mat4(pMat));
		}

		if (m_pSelectedEntity)
		{
			float* pMat = (float*)glm::value_ptr(m_pSelectedEntity->GetModelMatrix());
			ImGuizmo::Manipulate((const float*)glm::value_ptr(m_PerspectiveCamera.GetCamera().GetViewMatrix()), (const float*)glm::value_ptr(m_PerspectiveCamera.GetCamera().GetProjectionMatrix()), mCurrentGizmoOperation, mCurrentGizmoMode, pMat);
			m_pSelectedEntity->SetModelMatrix(glm::make_mat4(pMat));
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
						std::shared_ptr<Lamp::Texture2D> selected;
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

			if (mousePos.x < (windowPos.x + windowSize.x) && mousePos.x > windowPos.x &&
				mousePos.y < (windowPos.y + windowSize.y) && mousePos.y > windowPos.y)
			{
				mousePos -= windowPos;

				m_MouseHoverPos = mousePos;
				if (m_MousePressed && m_PerspectiveHover)
				{
					m_pSelectedBrush = nullptr;
					m_pSelectedEntity = nullptr;

					m_pSelectedBrush = Lamp::BrushManager::Get()->GetBrushFromPoint(m_PerspectiveCamera.ScreenToWorldCoords(mousePos, windowSize), m_PerspectiveCamera.GetCamera().GetPosition());
					if (!m_pSelectedBrush)
					{
						m_pSelectedEntity = Lamp::EntityManager::Get()->GetEntityFromPoint(m_PerspectiveCamera.ScreenToWorldCoords(mousePos, windowSize), m_PerspectiveCamera.GetCamera().GetPosition());
					}
				}
			}

			if (m_pSelectedEntity)
			{
				ImGui::Text("Entity");

				std::string name = m_pSelectedEntity->GetName();
				ImGui::InputText("Name", &name);
				m_pSelectedEntity->SetName(name);

				if (ImGui::CollapsingHeader("Transform"))
				{
					glm::vec3 pos = m_pSelectedEntity->GetPosition();
					float f[3] = { pos.x, pos.y, pos.z };

					ImGui::InputFloat3("Position", f);
					m_pSelectedEntity->SetPosition(glm::make_vec3(f));

					glm::vec3 rot = m_pSelectedEntity->GetRotation();
					float r[3] = { rot.x, rot.y, rot.z };

					ImGui::InputFloat3("Rotation", r);
					m_pSelectedEntity->SetRotation(glm::make_vec3(r));

					glm::vec3 scale = m_pSelectedEntity->GetScale();
					float s[3] = { scale.x, scale.y, scale.z };

					ImGui::InputFloat3("Scale", s);
					m_pSelectedEntity->SetScale(glm::make_vec3(s));
				}

				for (auto& pComp : m_pSelectedEntity->GetComponents())
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
								ImGui::InputText(pProp.Name.c_str(), s);
								break;
							}

							case Lamp::PropertyType::Color:
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
			}
			else if (m_pSelectedBrush)
			{
				ImGui::Text("Brush");

				std::string name = m_pSelectedBrush->GetName();
				ImGui::InputText("Name", &name);
				m_pSelectedBrush->SetName(name);

				if (ImGui::CollapsingHeader("Transform"))
				{
					glm::vec3 pos = m_pSelectedBrush->GetPosition();
					float f[3] = { pos.x, pos.y, pos.z };

					ImGui::InputFloat3("Position", f);
					m_pSelectedBrush->SetPosition(glm::make_vec3(f));

					glm::vec3 rot = m_pSelectedBrush->GetRotation();
					float r[3] = { rot.x, rot.y, rot.z };

					ImGui::InputFloat3("Rotation", r);
					m_pSelectedBrush->SetRotation(glm::make_vec3(r));

					glm::vec3 scale = m_pSelectedBrush->GetScale();
					float s[3] = { scale.x, scale.y, scale.z };

					ImGui::InputFloat3("Scale", s);
					m_pSelectedBrush->SetScale(glm::make_vec3(s));
				}

				if (ImGui::CollapsingHeader("Physics"))
				{
					glm::vec3 vel = m_pSelectedBrush->GetPhysicalEntity()->GetVelocity();
					float f[3] = { vel.x, vel.y, vel.z };

					ImGui::InputFloat3("Velocity", f);
					m_pSelectedBrush->GetPhysicalEntity()->SetVelocity(glm::make_vec3(f));

					float m = m_pSelectedBrush->GetPhysicalEntity()->GetMass();
					ImGui::InputFloat("Mass", &m);
					m_pSelectedBrush->GetPhysicalEntity()->SetMass(m);
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
			m_pShader = Lamp::Shader::Create("engine/shaders/shader_vs.glsl", "engine/shaders/shader_fs.glsl");
		}

		ImGui::Begin("Model importer", &m_ModelImporterOpen);

		static std::string path = "";
		static std::string savePath = "";

		if (ImGui::Button("Load"))
		{
			path = Lamp::FileSystem::GetFileFromDialogue();
			m_pModelToImport = Lamp::GeometrySystem::ImportModel(path);

			savePath = path.substr(0, path.find_last_of('.'));
			savePath += ".lgf";

			m_pModelToImport->GetMaterial().SetShader(m_pShader);

			m_pModelToImport->GetMaterial().SetDiffuse(Lamp::Texture2D::Create("engine/textures/default/defaultTexture.png"));
			m_pModelToImport->GetMaterial().SetSpecular(Lamp::Texture2D::Create("engine/textures/default/defaultTexture.png"));
		}
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
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
			ImGui::InputText("Diffuse path:", &diffPath);
			ImGui::SameLine();
			if (ImGui::Button("Load"))
			{
				diffPath = Lamp::FileSystem::GetFileFromDialogue();
			}
			m_pModelToImport->GetMaterial().SetDiffuse(Lamp::Texture2D::Create(diffPath));

			static std::string specPath = m_pModelToImport->GetMaterial().GetSpecular()->GetPath();
			ImGui::InputText("Specular path:", &specPath);
			ImGui::SameLine();
			if (ImGui::Button("Load"))
			{
				specPath = Lamp::FileSystem::GetFileFromDialogue();
			}
			m_pModelToImport->GetMaterial().SetSpecular(Lamp::Texture2D::Create(specPath));

			ImGui::InputText("Shader path:", &m_pModelToImport->GetMaterial().GetShader()->GetVertexPath());
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

		for (Lamp::ObjectLayer& layer : Lamp::ObjectLayerManager::Get()->GetLayers())
		{
			if (ImGui::Button("A"))
			{
				layer.IsActive = !layer.IsActive;
			}

			ImGui::SameLine();
			if (ImGui::TreeNode(layer.Name.c_str()))
			{

				for (int i = 0; i < layer.Objects.size(); i++)
				{
					if (ImGui::Button("A"))
					{
						layer.Objects[i]->SetIsActive(!layer.Objects[i]->GetIsActive());
					}

					ImGui::SameLine();

					startId++;
					ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					ImGui::TreeNodeEx((void*)(intptr_t)startId, nodeFlags, layer.Objects[i]->GetName().c_str());

				}

				ImGui::TreePop();
			}
		}

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
					Lamp::LevelSystem::SaveLevel("engine/levels/" + Lamp::LevelSystem::GetCurrentLevel()->GetName() + ".level", Lamp::LevelSystem::GetCurrentLevel());
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("Import Model", NULL, &m_ModelImporterOpen);
				ImGui::MenuItem("Properties", NULL, &m_InspectiorOpen);
				ImGui::MenuItem("Asset browser", NULL, &m_AssetBrowserOpen);
				ImGui::MenuItem("Layer view", NULL, &m_LayerViewOpen);

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