#include "Sandbox2D.h"

namespace Sandbox2D
{
	Sandbox2D::Sandbox2D()
		: m_SelectedFile("")
	{
		m_CameraController = new Lamp::OrthographicCameraController(Lamp::Application::Get().GetWindow().GetWidth() / Lamp::Application::Get().GetWindow().GetHeight());
	
		auto tempLevel = Lamp::LevelSystem::LoadLevel("assets/levels/2DLevel.level");
		auto brush = Lamp::BrushManager::Get()->Create2D("assets/textures/vlad.PNG");
		auto brush1 = Lamp::BrushManager::Get()->Create2D("assets/textures/ff.PNG");
		brush1->SetPosition({ 1.f, 0.f, 0.f });
	}

	void Sandbox2D::Update(Lamp::Timestep ts)
	{
		m_CameraController->Update(ts);
		GetInput();

		Lamp::PhysicsEngine::Get()->Simulate(ts);
		Lamp::PhysicsEngine::Get()->HandleCollisions();

		Lamp::AppUpdateEvent updateEvent(ts);
		Lamp::ObjectLayerManager::Get()->OnEvent(updateEvent);

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		Lamp::Renderer2D::ResetStats();
		Lamp::Renderer2D::Begin(m_CameraController->GetCamera());

		Lamp::AppRenderEvent renderEvent;
		Lamp::ObjectLayerManager::Get()->OnEvent(renderEvent);
		m_CameraController->OnEvent(renderEvent);

		Lamp::Renderer2D::End();
	}

	void Sandbox2D::OnImGuiRender(Lamp::Timestep ts)
	{
		UpdateDockspace();

		RenderProperties();
		RenderPerspective();
		RenderAssetBrowser();
		RenderLayerView();
	}

	void Sandbox2D::OnEvent(Lamp::Event& e)
	{
		m_CameraController->OnEvent(e);

		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::MouseMovedEvent>(LP_BIND_EVENT_FN(Sandbox2D::MouseMoved));
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