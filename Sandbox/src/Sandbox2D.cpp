#include "lppch.h"
#include "Sandbox2D.h"

#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Renderer3D.h"
#include <Lamp/Physics/Collision.h>
#include <Lamp/Brushes/BrushManager.h>
#include <Lamp/Level/LevelSystem.h>
#include <Lamp/Event/ApplicationEvent.h>

namespace Sandbox2D
{
	Sandbox2D::Sandbox2D()
		: Lamp::Layer("Sandbox2D"), m_SelectedFile(""), m_DockspaceID(0), m_PCam(45.f, 0.1f, 100.f)
	{
		auto tempLevel = Lamp::LevelSystem::LoadLevel("engine/levels/Level.level");
	}

	void Sandbox2D::Update(Lamp::Timestep ts)
	{
		m_PCam.Update(ts);
		Lamp::EntityManager::Get()->Update(ts);

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		Lamp::Renderer3D::Begin(m_PCam.GetCamera());

		Lamp::AppRenderEvent renderEvent;
		//Lamp::EntityManager::Get().OnEvent(renderEvent);
		Lamp::BrushManager::Get()->OnEvent(renderEvent);

		Lamp::Renderer3D::End();
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
		m_PCam.OnEvent(e);

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