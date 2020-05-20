#include "lppch.h"
#include "ConvexPolygonCollider.h"
#include <Lamp/Rendering/Renderer2D.h>

#include "Lamp/Entity/Base/Entity.h"

#include <Lamp/Entity/Base/ComponentRegistry.h>

namespace Lamp
{
	bool ConvexPolygonColliderComponent::s_Registered = LP_REGISTER_COMPONENT(ConvexPolygonColliderComponent);

	void ConvexPolygonColliderComponent::Initialize()
	{
	}

//	void ConvexPolygonColliderComponent::Update(Lamp::Timestep myTS)
//	{
//		for (size_t i = 0; i < m_VerticesModel.size(); i++)
//		{
//			m_VerticesWorld[i] =
//			{
//				(m_VerticesModel[i].x + m_pEntity->GetPosition().x),
//				(m_VerticesModel[i].y + m_pEntity->GetPosition().y)
//			};
//		}
//		m_IsColliding = false;
//	}
//
//	void ConvexPolygonColliderComponent::Draw()
//	{
//#ifdef LP_DEBUG
//
//		glm::vec3 currScale = m_pEntity->GetScale();
//		m_pEntity->SetScale({ 1.f, 1.f, 1.f });
//
//		for (size_t i = 0; i < m_VerticeCount; i++)
//		{
//			if (i == m_VerticeCount - 1)
//			{
//				Lamp::Renderer2D::DrawLine(m_VerticesWorld[i], m_VerticesWorld[0]);
//			}
//			else 
//			{
//				Lamp::Renderer2D::DrawLine(m_VerticesWorld[i], m_VerticesWorld[i + 1]);
//			}
//
//		}
//
//		m_pEntity->SetScale(currScale);
//
//		/*glBegin(GL_LINES);
//		glLineWidth(1.f);
//
//		for (size_t i = 0; i < m_VerticeCount; i++)
//		{
//			glLineWidth(1.f);
//			glVertex2f(m_VerticesWorld[i].x, m_VerticesWorld[i].y);
//
//			if (i == m_VerticeCount - 1)
//			{
//				glVertex2f(m_VerticesWorld[0].x, m_VerticesWorld[0].y);
//			}
//			else
//			{
//				glVertex2f(m_VerticesWorld[i + 1].x, m_VerticesWorld[i + 1].y);
//			}
//		}
//		glEnd();*/
//#endif
//	}
}