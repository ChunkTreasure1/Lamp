#include "Sandbox2D.h"

namespace Sandbox2D
{
	Sandbox2D::Sandbox2D()
	{
		m_CameraController = new Lamp::OrthographicCameraController(Lamp::Application::Get().GetWindow().GetWidth() / Lamp::Application::Get().GetWindow().GetHeight());
	
		auto tempLevel = Lamp::LevelSystem::LoadLevel("assets/levels/2DLevel.level");

		auto brush = Lamp::BrushManager::Get()->Create2D("assets/textures/vlad.PNG");
	}

	void Sandbox2D::Update(Lamp::Timestep ts)
	{
		m_CameraController->Update(ts);

		Lamp::PhysicsEngine::Get()->Simulate(ts);
		Lamp::PhysicsEngine::Get()->HandleCollisions();

		Lamp::AppUpdateEvent updateEvent(ts);
		Lamp::ObjectLayerManager::Get()->OnEvent(updateEvent);

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		Lamp::Renderer2D::ResetStats();
		Lamp::Renderer2D::Begin(m_CameraController->GetCamera());

		//Lamp::Renderer2D::DrawQuad({ 0.f, 0.f, 0.f }, { 1.f, 1.f }, { 1.f, 1.f, 1.f, 1.f });

		Lamp::AppRenderEvent renderEvent;
		Lamp::ObjectLayerManager::Get()->OnEvent(renderEvent);
		m_CameraController->OnEvent(renderEvent);

		Lamp::Renderer2D::End();
	}

	void Sandbox2D::OnImGuiRender(Lamp::Timestep ts)
	{
	}

	void Sandbox2D::OnEvent(Lamp::Event& e)
	{
		m_CameraController->OnEvent(e);
	}

	void Sandbox2D::OnItemClicked(Lamp::File& file)
	{
	}
}