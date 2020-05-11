#include "lppch.h"
#include "Sandbox3D.h"

#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Renderer3D.h"
#include <Lamp/Physics/Collision.h>
#include <Lamp/Brushes/BrushManager.h>
#include <Lamp/Level/LevelSystem.h>
#include <Lamp/Event/ApplicationEvent.h>

namespace Sandbox3D
{
	Sandbox3D::Sandbox3D()
		: Lamp::Layer("Sandbox3D"), m_SelectedFile(""), m_DockspaceID(0), m_PCam(45.f, 0.1f, 100.f), m_pShader(nullptr)
	{
		auto tempLevel = Lamp::LevelSystem::LoadLevel("engine/levels/Level.level");
	}

	void Sandbox3D::Update(Lamp::Timestep ts)
	{
		m_PCam.Update(ts);
		GetInput();
		Lamp::EntityManager::Get()->Update(ts);

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		Lamp::Renderer3D::Begin(m_PCam.GetCamera());

		//sLamp::Renderer3D::DrawSphere();
		Lamp::AppRenderEvent renderEvent;
		//Lamp::EntityManager::Get()->OnEvent(renderEvent);
		Lamp::BrushManager::Get()->OnEvent(renderEvent);
		m_PCam.OnEvent(renderEvent);

		Lamp::Renderer3D::End();
	}

	void Sandbox3D::OnImGuiRender(Lamp::Timestep ts)
	{
		//CreateDockspace();	

		//UpdatePerspective();
		//UpdateAssetBrowser();
		//UpdateProperties();
		UpdateModelImporter();
	}

	void Sandbox3D::OnEvent(Lamp::Event& e)
	{
		m_PCam.OnEvent(e);

		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::MouseMovedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnMouseMoved));
	}

	void Sandbox3D::OnItemClicked(Lamp::File& file)
	{
		m_SelectedFile = file;
	}

	void Sandbox3D::GetInput()
	{
		if (Lamp::Input::IsMouseButtonPressed(0))
		{
			m_PCam.ScreenToWorldCoords(m_MouseHoverPos, glm::vec2(Lamp::Application::Get().GetWindow().GetWidth(), Lamp::Application::Get().GetWindow().GetHeight()));

			m_MousePressed = true;
		}
		else if (Lamp::Input::IsMouseButtonReleased(0))
		{
			m_MousePressed = false;
		}
	}

	void Sandbox3D::RenderGrid()
	{

	}
}