#include "Sandbox2D.h"

namespace Sandbox2D
{
	Sandbox2D::Sandbox2D()
		: m_SelectedFile("")
	{
		m_CameraController = new Lamp::OrthographicCameraController((float)Lamp::Application::Get().GetWindow().GetWidth() / (float)Lamp::Application::Get().GetWindow().GetHeight());
	
		auto tempLevel = Lamp::LevelSystem::LoadLevel("assets/levels/2DLevel.level");
		auto brush = Lamp::BrushManager::Get()->Create2D("assets/textures/vlad.PNG");
		auto brush1 = Lamp::BrushManager::Get()->Create2D("assets/textures/ff.PNG");
		brush1->SetPosition({ 1.f, 0.f, 0.f });

		auto ent = Lamp::EntityManager::Get()->Create();

		Lamp::ObjectLayerManager::Get()->AddLayer(Lamp::ObjectLayer(1, "Test", true));
		Lamp::ObjectLayerManager::Get()->MoveToLayer(ent, 1);
	}

	bool Sandbox2D::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		m_CameraController->Update(e.GetTimestep());
		GetInput();

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		Lamp::Renderer2D::ResetStats();
		Lamp::Renderer2D::Begin(m_CameraController->GetCamera());

		Lamp::AppRenderEvent renderEvent;
		Lamp::ObjectLayerManager::Get()->OnEvent(renderEvent);
		m_CameraController->OnEvent(renderEvent);

		Lamp::Renderer2D::End();

		return true;
	}

	void Sandbox2D::OnImGuiRender(Lamp::Timestep ts)
	{
		UpdateDockspace();

		RenderProperties();
		RenderPerspective();
		RenderAssetBrowser();
		RenderLayerView();

		ImGui::ShowDemoWindow();
	}

	void Sandbox2D::OnEvent(Lamp::Event& e)
	{
		m_CameraController->OnEvent(e);

		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::MouseMovedEvent>(LP_BIND_EVENT_FN(Sandbox2D::MouseMoved));
		dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(Sandbox2D::OnUpdate));
	}

	void Sandbox2D::OnItemClicked(Lamp::File& file)
	{
	}

	void Sandbox2D::GetInput()
	{
		if (Lamp::Input::IsMouseButtonPressed(0))
		{
			m_MousePressed = true;
		}
		else if (Lamp::Input::IsMouseButtonReleased(0))
		{
			m_MousePressed = false;
		}
	}
}