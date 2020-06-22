#include "Sandbox2D.h"

namespace Sandbox2D
{
	Sandbox2D::Sandbox2D()
	{
		Lamp::Renderer2D::Initialize();

		m_CameraController = new Lamp::OrthographicCameraController(Lamp::Application::Get().GetWindow().GetWidth() / Lamp::Application::Get().GetWindow().GetHeight());
	}

	void Sandbox2D::Update(Lamp::Timestep ts)
	{
		Lamp::Renderer2D::ResetStats();

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		Lamp::Renderer2D::Begin(m_CameraController->GetCamera());

		Lamp::Renderer2D::End();
	}

	void Sandbox2D::OnImGuiRender(Lamp::Timestep ts)
	{
	}

	void Sandbox2D::OnEvent(Lamp::Event& e)
	{
	}

	void Sandbox2D::OnItemClicked(Lamp::File& file)
	{
	}
}