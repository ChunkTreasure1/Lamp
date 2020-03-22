#include "lppch.h"
#include "Sandbox2D.h"

#include "Lamp/Rendering/Renderer2D.h"
#include <Lamp/Physics/Collision.h>
#include <Lamp/Brushes/BrushManager.h>
#include <Lamp/Level/LevelSystem.h>

namespace Sandbox2D
{
	Sandbox2D::Sandbox2D()
		: Lamp::Layer("Sandbox2D"), m_CameraController(m_AspectRatio), m_SelectedFile(""), m_DockspaceID(0)
	{
		m_FrameBuffer = Lamp::FrameBuffer::Create(1280, 720);
		auto tempLevel = Lamp::LevelSystem::LoadLevel("engine/levels/Level.level");
	}

	void Sandbox2D::Update(Lamp::Timestep ts)
	{
		m_CameraController.Update(ts);
		Lamp::EntityManager::Get().Update(ts);

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		m_FrameBuffer->Bind();
		Lamp::Renderer::Clear();

		Lamp::Renderer2D::Begin(m_CameraController.GetCamera());

		Lamp::BrushManager::Get().Draw();
		Lamp::EntityManager::Get().Draw();

		Lamp::Renderer2D::End();
		m_FrameBuffer->Unbind();
	}
	 
	void Sandbox2D::OnImGuiRender(Lamp::Timestep ts)
	{
		CreateDockspace();	

		UpdatePerspective();
		UpdateAssetBrowser();
		UpdateProperties();
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
}