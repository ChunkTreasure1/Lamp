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

namespace Sandbox3D
{
	Sandbox3D::Sandbox3D()
		: Lamp::Layer("Sandbox3D"), m_SelectedFile(""), m_DockspaceID(0), m_PCam(60.f, 0.1f, 100.f), m_pShader(nullptr)
	{
		auto tempLevel = Lamp::LevelSystem::LoadLevel("engine/levels/Level.level");

		std::shared_ptr<Lamp::BoundingSphere> sphere1 = std::make_shared<Lamp::BoundingSphere>(glm::vec3(0.f, 0.f, 0.f), 1.f);
		std::shared_ptr<Lamp::BoundingSphere> sphere2 = std::make_shared<Lamp::BoundingSphere>(glm::vec3(0.f, 3.f, 0.f), 1.f);
		std::shared_ptr<Lamp::BoundingSphere> sphere3 = std::make_shared<Lamp::BoundingSphere>(glm::vec3(0.f, 0.f, 2.f), 1.f);
		std::shared_ptr<Lamp::BoundingSphere> sphere4 = std::make_shared<Lamp::BoundingSphere>(glm::vec3(1.f, 0.f, 0.f), 1.f);

		Lamp::IntersectData intersect1 = sphere1->IntersectBoundingSphere(sphere2);
		Lamp::IntersectData intersect2 = sphere1->IntersectBoundingSphere(sphere3);
		Lamp::IntersectData intersect3 = sphere1->IntersectBoundingSphere(sphere4);

		std::string int1 = std::to_string(intersect1.IsIntersecting) + ": " + std::to_string(intersect1.Distance);
		std::string int2 = std::to_string(intersect2.IsIntersecting) + ": " + std::to_string(intersect2.Distance);
		std::string int3 = std::to_string(intersect3.IsIntersecting) + ": " + std::to_string(intersect3.Distance);

		LP_INFO(int1);
		LP_INFO(int2);
		LP_INFO(int3);

		std::shared_ptr<Lamp::AABB> ab1 = std::make_shared<Lamp::AABB>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f));
		std::shared_ptr<Lamp::AABB> ab2 = std::make_shared<Lamp::AABB>(glm::vec3(1.f, 1.f, 1.f), glm::vec3(2.f, 2.f, 2.f));
		std::shared_ptr<Lamp::AABB> ab3 = std::make_shared<Lamp::AABB>(glm::vec3(1.f, 0.f, 0.f), glm::vec3(2.f, 1.f, 1.f));
		std::shared_ptr<Lamp::AABB> ab4 = std::make_shared<Lamp::AABB>(glm::vec3(0.f, 0.f, -2.f), glm::vec3(1.f, 1.f, -1.f));
		std::shared_ptr<Lamp::AABB> ab5 = std::make_shared<Lamp::AABB>(glm::vec3(0.f, 0.5f, 0.f), glm::vec3(1.f, 1.5f, 1.f));
	
		Lamp::IntersectData intersect4 = ab1->IntersectAABB(ab2);
		Lamp::IntersectData intersect5 = ab1->IntersectAABB(ab3);
		Lamp::IntersectData intersect6 = ab1->IntersectAABB(ab4);
		Lamp::IntersectData intersect7 = ab1->IntersectAABB(ab5);

		std::string int4 = std::to_string(intersect4.IsIntersecting) + ": " + std::to_string(intersect4.Distance);
		std::string int5 = std::to_string(intersect5.IsIntersecting) + ": " + std::to_string(intersect5.Distance);
		std::string int6 = std::to_string(intersect6.IsIntersecting) + ": " + std::to_string(intersect6.Distance);
		std::string int7 = std::to_string(intersect7.IsIntersecting) + ": " + std::to_string(intersect7.Distance);
	
		LP_INFO(int4);
		LP_INFO(int5);
		LP_INFO(int6);
		LP_INFO(int7);
	}

	void Sandbox3D::Update(Lamp::Timestep ts)
	{
		m_PCam.Update(ts);
		GetInput();
		Lamp::EntityManager::Get()->Update(ts);

		Lamp::Renderer::SetClearColor(m_ClearColor);
		Lamp::Renderer::Clear();

		Lamp::Renderer3D::Begin(m_PCam.GetCamera());

		Lamp::AppRenderEvent renderEvent;
		//Lamp::EntityManager::Get()->OnEvent(renderEvent);
		Lamp::BrushManager::Get()->OnEvent(renderEvent);
		m_PCam.OnEvent(renderEvent);
		RenderGrid();

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
			Lamp::Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, - 0.5f * (float)x), glm::vec3(5.f, 0.f, -0.5f * (float)x), 1.f);
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