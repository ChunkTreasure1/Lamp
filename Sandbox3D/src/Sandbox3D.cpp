#include "lppch.h"
#include "Sandbox3D.h"

#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Renderer3D.h"
#include <Lamp/Physics/Collision.h>
#include <Lamp/Brushes/BrushManager.h>
#include <Lamp/Level/LevelSystem.h>
#include <Lamp/Event/ApplicationEvent.h>

#include <Lamp/Physics/Colliders/BoundingSphere.h>
#include <Lamp/Physics/Colliders/AABB.h>
#include <Lamp/Physics/PhysicsEngine.h>
#include <Lamp/Physics/Physics.h>

namespace Sandbox3D
{
	Sandbox3D::Sandbox3D()
		: Lamp::Layer("Sandbox3D"), m_SelectedFile(""), m_DockspaceID(0), m_PCam(60.f, 0.1f, 100.f), m_pShader(nullptr)
	{
		auto tempLevel = Lamp::LevelSystem::LoadLevel("engine/levels/Level.level");

		//auto brush1 = Lamp::BrushManager::Get()->Create("engine/models/test.lgf");
		//auto brush2 = Lamp::BrushManager::Get()->Create("engine/models/test.lgf");
		auto light = Lamp::BrushManager::Get()->Create("engine/models/lightModel.lgf");

		//brush1->SetPosition({ -10, 1, 0 });
		//brush2->SetPosition({ 10, 1.5, 0 });
		light->SetPosition({ 0, 7, 0 });

		//Lamp::PhysicsEngine::Get()->AddEntity(brush1->GetPhysicalEntity());
		//Lamp::PhysicsEngine::Get()->AddEntity(brush2->GetPhysicalEntity());
	}

	void Sandbox3D::Update(Lamp::Timestep ts)
	{
		m_PCam.Update(ts);
		GetInput();

		Lamp::PhysicsEngine::Get()->Simulate(ts);
		Lamp::PhysicsEngine::Get()->HandleCollisions();

		Lamp::EntityManager::Get()->Update(ts);

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		Lamp::Renderer3D::Begin(m_PCam.GetCamera());

		Lamp::AppRenderEvent renderEvent;
		//Lamp::EntityManager::Get()->OnEvent(renderEvent);
		Lamp::BrushManager::Get()->OnEvent(renderEvent);
		m_PCam.OnEvent(renderEvent);
		RenderGrid();

		Lamp::Renderer3D::DrawSkybox();
		Lamp::Renderer3D::End();
	}

	void Sandbox3D::OnImGuiRender(Lamp::Timestep ts)
	{
		//CreateDockspace();	

		//UpdatePerspective();
		UpdateAssetBrowser();
		UpdateProperties();
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
			m_pSelectedBrush = Lamp::BrushManager::Get()->GetBrushFromPoint(m_PCam.ScreenToWorldCoords(m_MouseHoverPos, glm::vec2(Lamp::Application::Get().GetWindow().GetWidth(), Lamp::Application::Get().GetWindow().GetHeight())), m_PCam.GetCamera().GetPosition());
			m_pSelectedEntity = nullptr;

			m_MousePressed = true;
		}
		else if (Lamp::Input::IsMouseButtonReleased(0))
		{
			m_MousePressed = false;
		}
	}

	void Sandbox3D::RenderGrid()
	{
		//glm::vec3 pos;
		//if (m_pSelectedBrush)
		//{
		//	pos = m_pSelectedBrush->GetPosition();
		//}
		//else if (m_pSelectedEntity)
		//{
		//	pos = m_pSelectedEntity->GetPosition();
		//}
		//else 
		//{
		//	return;
		//}

		//for (size_t z = 0; z < 10; z++)
		//{
		//	Lamp::Renderer3D::DrawLine(glm::vec3(pos.x - 5.f, pos.y, pos.z + 0.5f * (float)z - 1.25f), glm::vec3(pos.x + 5.f, pos.y, pos.z + 0.5f * (float)z - 1.25f), 1.f);
		//}

		//for (size_t x = 0; x < 10; x++)
		//{
		//	Lamp::Renderer3D::DrawLine(glm::vec3(pos.x + 0.5f * (float)x - 1.5f, pos.y, pos.z - 5.f), glm::vec3(pos.x + 0.5f * (float)x - 1.5f, pos.y, pos.z + 5.f), 1.f);
		//}

		Lamp::Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, 0.f), glm::vec3(5.f, 0.f, 0.f), 1.f);
		Lamp::Renderer3D::DrawLine(glm::vec3(0.f, 0.f, -5.f), glm::vec3(0.f, 0.f, 5.f), 1.f);

		for (size_t x = 1; x <= 10; x++)
		{
			Lamp::Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, 0.5f * (float)x), glm::vec3(5.f, 0.f, 0.5f * (float)x), 1.f);
		}

		for (size_t x = 1; x <= 10; x++)
		{
			Lamp::Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, -0.5f * (float)x), glm::vec3(5.f, 0.f, -0.5f * (float)x), 1.f);
		}

		for (size_t z = 1; z <= 10; z++)
		{
			Lamp::Renderer3D::DrawLine(glm::vec3(0.5f * (float)z, 0.f, -5.f), glm::vec3(0.5f * (float)z, 0.f, 5.f), 1.f);
		}

		for (size_t z = 1; z <= 10; z++)
		{
			Lamp::Renderer3D::DrawLine(glm::vec3(-0.5f * (float)z, 0.f, -5.f), glm::vec3(-0.5f * (float)z, 0.f, 5.f), 1.f);
		}
	}
}