#include "lppch.h"
#include "Sandbox2D.h"

namespace Sandbox2D
{
	void Sandbox2D::UpdatePerspective()
	{
		ImGui::Begin("Perspective");
		{
			m_PerspectiveHover = ImGui::IsWindowHovered();
			m_CameraController.SetHasControl(m_PerspectiveHover);

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Aspect ratio"))
				{

				}
				ImGui::EndMenuBar();
			}

			m_CameraController.SetAspectRatio(m_AspectRatio);
			ImVec2 pos = ImGui::GetCursorScreenPos();

			float height = ImGui::GetWindowSize().x / m_AspectRatio;

			float offset = (ImGui::GetWindowSize().y - height) / 2;

			m_FrameBuffer->Update((uint32_t)height, (uint32_t)ImGui::GetWindowSize().y);

			ImGui::GetWindowDrawList()->AddImage((void*)(uint64_t)m_FrameBuffer->GetTexture(),
				ImVec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + offset),
				ImVec2(pos.x + ImGui::GetWindowSize().x, pos.y + ImGui::GetWindowSize().y - offset),
				ImVec2(0, 1),
				ImVec2(1, 0));
		}
		ImGui::End();
	}

	void Sandbox2D::UpdateAssetBrowser()
	{
		ImGui::Begin("Asset Browser");
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
					if (m_SelectedFile.GetFileType() == Lamp::FileType_Texture)
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

	void Sandbox2D::UpdateProperties()
	{
		ImGui::Begin("Properties");
		{
			if (m_MousePressed)
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


				if (mousePos.x < (windowPos.x + windowSize.x) && mousePos.x > windowPos.x&&
					mousePos.y < (windowPos.y + windowSize.y) && mousePos.y > windowPos.y)
				{
					mousePos -= windowPos;

					for (Lamp::IEntity* pEnt : m_pEntityManager->GetEntities())
					{
						if (Lamp::IEntity* pEnt = Lamp::EntityManager::GetEntityFromPoint(m_CameraController.ScreenToWorldCoords(mousePos, windowSize), m_pEntityManager))
						{
							m_pSelectedEntity = pEnt;
						}
						else
						{
							m_pSelectedEntity = nullptr;
						}
					}
				}
			}

			if (m_pSelectedEntity)
			{
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

									pComp->SetProperty(pProp, &p);
									break;
								}

								case Lamp::PropertyType::Bool:
								{
									bool* p = static_cast<bool*>(pProp.Value);
									ImGui::Checkbox(pProp.Name.c_str(), p);

									pComp->SetProperty(pProp, &p);
									break;
								}

								case Lamp::PropertyType::Float2:
								{
									glm::vec2* p = static_cast<glm::vec2*>(pProp.Value);

									float f[2] = { p->x, p->y };
									ImGui::InputFloat2(pProp.Name.c_str(), f, 3);

									pComp->SetProperty(pProp, &f);
									break;
								}

								case Lamp::PropertyType::Float3:
								{
									glm::vec3* p = static_cast<glm::vec3*>(pProp.Value);

									float f[3] = { p->x, p->y, p->z };
									ImGui::InputFloat3(pProp.Name.c_str(), f, 3);

									pComp->SetProperty(pProp, &f);
									break;
								}

								case Lamp::PropertyType::Float4:
								{
									glm::vec4* p = static_cast<glm::vec4*>(pProp.Value);
									
									float f[4] = { p->x, p->y, p->z, p->w };
									ImGui::InputFloat4(pProp.Name.c_str(), f, 3);

									//Change to only send name
									pComp->SetProperty(pProp, &f);
									break;
								}

								case Lamp::PropertyType::String:
								{
									std::string* s = static_cast<std::string*>(pProp.Value);
									char* buf = new char[s->size() + 1];
									strcpy(buf, s->c_str());
									ImGui::InputText(pProp.Name.c_str(), buf, s->size());

									pComp->SetProperty(pProp, &buf);
									break;
								}

								case Lamp::PropertyType::Color:
								{
									glm::vec4* p = static_cast<glm::vec4*>(pProp.Value);

									float f[4] = { p->x, p->y, p->z, p->w };
									ImGui::ColorEdit4(pProp.Name.c_str(), f);

									pComp->SetProperty(pProp, &f);
									break;
								}
							}
						}
					}
				}
			}
		}
		ImGui::End();
	}

	void Sandbox2D::CreateDockspace()
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
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0))                 dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();
	}

	bool Sandbox2D::OnMouseMoved(Lamp::MouseMovedEvent& e)
	{
		m_MouseHoverPos = glm::vec2(e.GetX(), e.GetY());

		return true;
	}
}