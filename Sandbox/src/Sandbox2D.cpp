#include "lppch.h"
#include "Sandbox2D.h"

#include "Lamp/Rendering/Renderer2D.h"

namespace Sandbox2D
{
	Sandbox2D::Sandbox2D()
		: Lamp::Layer("Sandbox2D"), m_CameraController(m_AspectRatio), m_SelectedFile(""), m_DockspaceID(0)
	{
		m_pEntityManager = new Lamp::EntityManager();
		m_pEntity = m_pEntityManager->CreateEntity(glm::vec3(0.f, 0.f, 0.f), "Assets/Textures/ff.PNG");

		m_FrameBuffer = Lamp::FrameBuffer::Create(1280, 720);
	}

	void Sandbox2D::Update(Lamp::Timestep ts)
	{
		m_CameraController.Update(ts);
		m_pEntityManager->Update();

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		m_FrameBuffer->Bind();
		Lamp::Renderer::Clear();

		Lamp::Renderer2D::Begin(m_CameraController.GetCamera());

		m_pEntityManager->Draw();

		//Lamp::Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, m_pTestTexture);

		Lamp::Renderer2D::End();
		m_FrameBuffer->Unbind();
	}

	void Sandbox2D::OnImGuiRender(Lamp::Timestep ts)
	{
		CreateDockspace();

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
						//Lamp::GLTexture selected = Lamp::ResourceManager::GetTexture(m_SelectedFile.GetPath());
						//ImGui::Image((void*)(uint64_t)selected.Id, ImVec2(ImGui::GetWindowSize().y * 0.9f, ImGui::GetWindowSize().y * 0.9f));
					}
				}
				ImGui::EndChild();
			}
		}
		ImGui::End();

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


				if (mousePos.x < (windowPos.x + windowSize.x) && mousePos.x > windowPos.x &&
					mousePos.y < (windowPos.y + windowSize.y) && mousePos.y > windowPos.y)
				{
					mousePos -= windowPos;

					glm::vec2 pos = m_CameraController.ScreenToWorldCoords(mousePos, windowSize);
					LP_CORE_INFO(std::to_string(pos.x) + ", " + std::to_string(pos.y));

					//for (Lamp::IEntity* pEnt : m_pEntityManager->GetEntities())
					//{
					//	if (auto pEnt = GetEntityFromPoint(m_CameraController.ScreenToWorldCoords(mousePos)))
					//	{
					//		for (auto pComp : pEnt->GetComponents())
					//		{
					//			//pComp->GetEditorValues();
					//		}
					//	}
					//}
				}
			}
		}
		ImGui::End();
	}

	void Sandbox2D::OnEvent(Lamp::Event& e)
	{
		m_CameraController.OnEvent(e);

		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::MouseMovedEvent>(LP_BIND_EVENT_FN(Sandbox2D::OnMouseMoved));

		if (e.GetEventType() == Lamp::EventType::MouseButtonPressed)
		{
			m_MousePressed = true;
		}
		else if (e.GetEventType() == Lamp::EventType::MouseButtonReleased)
		{
			m_MousePressed = false;
		}
	}

	void Sandbox2D::OnItemClicked(Lamp::File& file)
	{
		m_SelectedFile = file;
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

	Lamp::IEntity* Sandbox2D::GetEntityFromPoint(glm::vec2& pos)
	{
		for (Lamp::IEntity* pEnt : m_pEntityManager->GetEntities())
		{
			if (auto pTrans = pEnt->GetComponent<Lamp::TransformComponent>())
			{
				//glm::vec4 rect(pTrans->GetPosition(), 1 * pTrans->GetScale(), 1 * pTrans->GetScale());

				//LP_CORE_INFO(std::to_string(rect.x) + ", " + std::to_string(rect.y) + ", " + std::to_string(rect.z) + ", " + std::to_string(rect.w));


				//if (pos.x > rect.x&&
				//	pos.x < rect.x + rect.z &&
				//	pos.y > rect.y&&
				//	pos.y < rect.y + rect.w)
				//{
				//	return pEnt;
				//}
			}
		}

		return nullptr;
	}
}