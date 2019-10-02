#include <Lamp.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "Lamp/Entity/BaseComponents/TransformComponent.h"
#include "Lamp/Entity/Base/EntityManager.h"

#include "Lamp/Event/Event.h"

#include <glm/gtc/type_ptr.hpp>

class RenderLayer : public Lamp::Layer
{
public:
	RenderLayer()
		: Layer("Render Layer"), m_CameraController(m_AspectRatio)
	{
		m_pEntityManager = new Lamp::EntityManager();
		m_pEntity = m_pEntityManager->CreateEntity(glm::vec2(0, 0), "Assets/Textures/ff.PNG");
		m_pEntity2 = m_pEntityManager->CreateEntity(glm::vec2(1, 0), "Assets/Textures/ff.PNG");

		m_pShader.reset(new Lamp::Shader("Assets/Shaders/colorShading.vert", "Assets/Shaders/colorShading.frag"));

		m_pShader->AddAttribute("vertexPosition");
		m_pShader->AddAttribute("vertexColor");
		m_pShader->AddAttribute("vertexUV");

		m_pShader->LinkShaders();

		Lamp::Renderer::GenerateFrameBuffers(m_FBO);
		Lamp::Renderer::CreateTexture(m_FBOTexture);
	}
	~RenderLayer()
	{
		m_pEntityManager->Shutdown();
		delete m_pEntityManager;
	}

	virtual void Update(Lamp::Timestep ts) override
	{
		m_CameraController.Update(ts);

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		Lamp::Renderer::BindFBO(m_FBO);
		Lamp::Renderer::Clear();

		Lamp::Renderer::Begin(m_CameraController.GetCamera());

		Lamp::Renderer::Draw(m_pShader, m_pEntity);
		Lamp::Renderer::Draw(m_pShader, m_pEntity2);

		Lamp::Renderer::End();

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FBOTexture, 0);
		Lamp::Renderer::UnbindFBO();
	}

	virtual void OnImGuiRender(Lamp::Timestep ts) override
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

			ImGui::GetWindowDrawList()->AddImage((void*)m_FBOTexture,
				ImVec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y + offset),
				ImVec2(pos.x + ImGui::GetWindowSize().x, pos.y + ImGui::GetWindowSize().y - offset),
				ImVec2(0, 1),
				ImVec2(1, 0));
		}
		ImGui::End();

		ImGui::Begin("Properties");
		{
			if (m_MousePressed && m_PerspectiveHover)
			{
				if (auto pEnt = GetEntityFromPoint(m_MouseHoverPos))
				{
					LP_CORE_INFO("HIT");
				}
			}
		}
		ImGui::End();
	}

	virtual void OnEvent(Lamp::Event& e) override
	{
		m_CameraController.OnEvent(e);
		
		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::MouseMovedEvent>(LP_BIND_EVENT_FN(RenderLayer::OnMouseMoved));

		if (e.GetEventType() == Lamp::EventType::MouseButtonPressed)
		{
			m_MousePressed = true;
		}
		else if (e.GetEventType() == Lamp::EventType::MouseButtonReleased)
		{
			m_MousePressed = false;
		}
	}

	void CreateDockspace()
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
		bool *p = &pp;
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

	bool OnMouseMoved(Lamp::MouseMovedEvent& e)
	{
		m_MouseHoverPos = glm::vec2(e.GetX(), e.GetY());
		return true;
	}

	Lamp::IEntity* GetEntityFromPoint(const glm::vec2& pos)
	{
		for (Lamp::IEntity* pEnt : m_pEntityManager->GetEntities())
		{
			if (auto pTrans = pEnt->GetComponent<Lamp::TransformComponent>())
			{
				glm::vec4 rect(pTrans->GetPosition(), 1 * pTrans->GetScale(), 1 * pTrans->GetScale());

				if (pos.x > rect.x &&
					pos.x < rect.x + rect.z &&
					pos.y > rect.y &&
					pos.y < rect.y + rect.w);
				{
					return pEnt;
				}
			}
		}

		return nullptr;
	}

private:
	Lamp::OrthographicCameraController m_CameraController;
	std::shared_ptr<Lamp::Shader> m_pShader;

	Lamp::IEntity* m_pEntity;
	Lamp::IEntity* m_pEntity2;
	Lamp::EntityManager* m_pEntityManager;

	Lamp::IEntity* m_pSelectedEntity = nullptr;

	glm::vec3 m_FColor = glm::vec3{ 0.1f, 0.1f, 0.1f };
	glm::vec4 m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
	std::string title;
	ImGuiID m_DockspaceID;

	const float m_AspectRatio = 1.7f;
	bool m_MousePressed = false;
	bool m_PerspectiveHover = false;

	glm::vec2 m_MouseHoverPos = glm::vec2();

	uint32_t m_FBO;
	uint32_t m_FBOTexture;
	int m_CurrSample = -1;
};

class UILayer : public Lamp::Layer
{
public:
	UILayer()
		: Lamp::Layer("UI Layer"), m_SelectedFile("")
	{}

	virtual void OnImGuiRender(Lamp::Timestep ts) override
	{
		ImGui::Begin("Asset Browser");
		{
			//Asset browser
			{
				ImGui::BeginChild("Browser", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.12, ImGui::GetWindowSize().y * 0.85), true);
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
						Lamp::GLTexture selected = Lamp::ResourceManager::GetTexture(m_SelectedFile.GetPath());
						ImGui::Image((void *)selected.Id, ImVec2(ImGui::GetWindowSize().y * 0.9f, ImGui::GetWindowSize().y * 0.9f));
					}
				}
				ImGui::EndChild();
			}
		}
		ImGui::End();
	}

	virtual void OnItemClicked(Lamp::File& file) override
	{
		m_SelectedFile = file;
	}

private:
	Lamp::File m_SelectedFile;
};

class Sandbox : public Lamp::Application
{
public:
	Sandbox()
	{
		PushLayer(new RenderLayer());
		PushLayer(new UILayer());
	};
	~Sandbox() {};
};

Lamp::Application* Lamp::CreateApplication()
{
	return new Sandbox();
}