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

		static Lamp::GLTexture texture = Lamp::ResourceManager::GetTexture("Textures/vlad.PNG");
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
		ImGui::Begin("Color");

		if (ImGui::ColorEdit3("Background color", m_FColor))
		{
			m_ClearColor.r = m_FColor[0];
			m_ClearColor.g = m_FColor[1];
			m_ClearColor.b = m_FColor[2];
			m_ClearColor.a = 1.f;
		}

		ImGui::End();

		ImGui::Begin("Test");
		ImGui::Text("Hello");
		ImGui::End();

		if (ImGui::DockBuilderGetNode(m_DockspaceID) == NULL)
		{
			CreateDockspace();
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
		static bool fullscreenPersistant = true;
		bool fullscreen = fullscreenPersistant;
		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (fullscreen)
		{
			ImGuiViewport* pViewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(pViewport->Pos);
			ImGui::SetNextWindowSize(pViewport->Size);
			ImGui::SetNextWindowViewport(pViewport->ID);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		{
			windowFlags |= ImGuiWindowFlags_NoBackground;
		}

		bool t = true;
		bool* pOpen = &t;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		ImGui::Begin("Dockspace", pOpen, windowFlags);
		ImGui::PopStyleVar();

		if (fullscreen)
		{
			ImGui::PopStyleVar(2);
		}

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspaceID = ImGui::GetID("MyDockspace");
			ImGui::DockSpace(dockspaceID, ImVec2(0.f, 0.f), dockspaceFlags);

			m_DockspaceID = dockspaceID;
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