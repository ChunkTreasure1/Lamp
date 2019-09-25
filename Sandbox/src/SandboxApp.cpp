#include <Lamp.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

class RenderLayer : public Lamp::Layer
{
public:
	RenderLayer()
		: Layer("Render Layer"), m_CameraController(1280 / 720)
	{ 
		m_pShader.reset(new Lamp::Shader("Shaders/colorShading.vert", "Shaders/colorShading.frag"));

		m_pShader->AddAttribute("vertexPosition");
		m_pShader->AddAttribute("vertexColor");
		m_pShader->AddAttribute("vertexUV");

		m_pShader->LinkShaders();

		static Lamp::GLTexture texture = Lamp::ResourceManager::GetTexture("Textures/ff.PNG");
		glm::vec4 pos(10, 10, 50, 50);
		sprite = new Lamp::Sprite(pos, texture.Id, 0.f);
	}

	void Update(Lamp::Timestep ts) override
	{
		m_CameraController.Update(ts);

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		Lamp::Renderer::Begin(m_CameraController.GetCamera());

		Lamp::Renderer::Draw(m_pShader, *sprite);

		Lamp::Renderer::End();
	}

	virtual void OnImGuiRender() override
	{
		CreateDockspace();

		ImGui::Begin("Color");

		if (ImGui::ColorEdit3("Background color", m_FColor))
		{
			m_ClearColor.r = m_FColor[0];
			m_ClearColor.g = m_FColor[1];
			m_ClearColor.b = m_FColor[2];
			m_ClearColor.a = 1.f;
		}

		ImGui::End();

		ImGui::Begin("Scene");

		ImGui::Image((void*)(intptr_t)sprite->GetTexuture(), ImVec2(100, 100));

		ImGui::End();

		ImGui::Begin("Test");
		ImGui::Text("Hello");
		ImGui::End();

		if (ImGui::DockBuilderGetNode(m_DockspaceID) == NULL)
		{
			SetEditorLayout();
		}
	}

	void OnEvent(Lamp::Event& e) override
	{
		m_CameraController.OnEvent(e);
	}

	void SetEditorLayout()
	{
		ImGui::DockBuilderRemoveNode(m_DockspaceID);
		ImGui::DockBuilderAddNode(m_DockspaceID, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(m_DockspaceID, ImVec2(1280, 720));

		ImGuiID dock_main_id = m_DockspaceID;
		ImGuiID dock_id_prop = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
		ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);
	
		ImGui::DockBuilderDockWindow("Color", dock_id_bottom);
		ImGui::DockBuilderDockWindow("Test", dock_id_prop);
		ImGui::DockBuilderFinish(m_DockspaceID);
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

private:
	Lamp::OrthographicCameraController m_CameraController;
	std::shared_ptr<Lamp::Shader> m_pShader;
	Lamp::Sprite* sprite;

	float m_FColor[3] = { 0.1f, 0.1f, 0.1f };
	glm::vec4 m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
	std::string title;
	ImGuiID m_DockspaceID;
};

class Sandbox : public Lamp::Application
{
public:
	Sandbox()
	{
		PushLayer(new RenderLayer());
	};
	~Sandbox() {};
};

Lamp::Application* Lamp::CreateApplication()
{
	return new Sandbox();
}