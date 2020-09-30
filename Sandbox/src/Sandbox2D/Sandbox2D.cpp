#include "Sandbox2D.h"

#include <Lamp/Rendering/RenderCommand.h>
#include <Game/Game.h>

namespace Sandbox2D
{
	Sandbox2D::Sandbox2D()
		: m_SelectedFile("")
	{
		m_CameraController = CreateRef<Lamp::OrthographicCameraController>((float)Lamp::Application::Get().GetWindow().GetWidth() / (float)Lamp::Application::Get().GetWindow().GetHeight());
		m_CameraController->SetPosition(glm::vec3(0.f, 0.f, 0.f));
		m_pGame = CreateScope<Game>();
		m_pGame->OnStart();
	}

	bool Sandbox2D::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		/*m_CameraController->Update(e.GetTimestep());
		GetInput();

		Lamp::RenderCommand::SetClearColor(m_ClearColor);
		Lamp::RenderCommand::Clear();

		Lamp::Renderer2D::ResetStats();
		Lamp::Renderer2D::Begin(m_CameraController->GetCamera());

		Lamp::AppRenderEvent renderEvent;
		Lamp::ObjectLayerManager::Get()->OnEvent(renderEvent);
		m_CameraController->OnEvent(renderEvent);

		Lamp::Renderer2D::End();*/

		return true;
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