#include "lppch.h"
#include "Entity.h"

#include "EntityManager.h"
#include "Lamp/Objects/ObjectLayer.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"

#include <glm/ext/matrix_transform.hpp>
#include "Lamp/GraphKey/GraphKeyGraph.h"

namespace Lamp
{
	void Entity::OnEvent(Event& e)
	{
		for (auto it = m_pComponents.begin(); it != m_pComponents.end(); it++)
		{
			if (m_pComponents.size() == 0)
			{
				return;
			}

			if (it->get()->GetEventMask() & e.GetEventType())
			{
				it->get()->OnEvent(e);
			}
		}

		if (m_GraphKeyGraph)
		{
			m_GraphKeyGraph->OnEvent(e);
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(Entity::OnRenderEvent));
	}

	void Entity::Destroy()
	{
		g_pEnv->pEntityManager->Remove(this);
		ObjectLayerManager::Get()->Remove(this);

		delete this;
	}

	Entity* Entity::Create()
	{
		return g_pEnv->pEntityManager->Create(false);
	}

	bool Entity::OnRenderEvent(AppRenderEvent& e)
	{
		if (g_pEnv->ShouldRenderGizmos)
		{
			if (!m_GizmoShader || !m_SelectionShader)
			{
				m_GizmoShader = ShaderLibrary::GetShader("Sprite");
				m_SelectionShader = ShaderLibrary::GetShader("selection");
			}

			if (!m_GizmoShader || !m_SelectionShader)
			{
				return false;
			}

			switch (e.GetPassInfo().type)
			{
			case PassType::Forward:
			{
				m_GizmoShader->Bind();

				m_GizmoShader->UploadMat4("u_ViewProjection", e.GetPassInfo().Camera->GetViewProjectionMatrix());

				glm::vec3 dir = glm::normalize(e.GetPassInfo().Camera->GetPosition() - m_Position);
				
				float angleXZ = std::atan2f(dir.z, dir.x);
				float angleY = -std::asin(dir.y);

				glm::mat4 rotation = glm::rotate(glm::mat4(1.f), -angleXZ + glm::radians(90.f), { 0.f, 1.f, 0.f })
					* glm::rotate(glm::mat4(1.f), angleY, { 1.f, 0.f, 0.f });

				glm::mat4 model = glm::translate(glm::mat4(1.f), m_Position)
					* rotation
					* glm::scale(glm::mat4(1.f), glm::vec3(0.5f));
				m_GizmoShader->UploadMat4("u_Model", model);
				m_GizmoShader->UploadInt("u_Texture", 0);

				m_GizmoTexure->Bind(0);

				Renderer3D::DrawQuad();
				break;
			}

			case PassType::Selection:
			{
				m_SelectionShader->Bind();

				m_SelectionShader->UploadMat4("u_ViewProjection", e.GetPassInfo().Camera->GetViewProjectionMatrix());

				glm::vec3 dir = glm::normalize(e.GetPassInfo().Camera->GetPosition() - m_Position);

				float angleXZ = std::atan2f(dir.z, dir.x);
				float angleY = -std::asin(dir.y);

				glm::mat4 rotation = glm::rotate(glm::mat4(1.f), -angleXZ + glm::radians(90.f), { 0.f, 1.f, 0.f })
					* glm::rotate(glm::mat4(1.f), angleY, { 1.f, 0.f, 0.f });

				glm::mat4 model = glm::translate(glm::mat4(1.f), m_Position)
					* rotation
					* glm::scale(glm::mat4(1.f), glm::vec3(0.5f));
				m_SelectionShader->UploadMat4("u_Model", model);
				m_SelectionShader->UploadInt("u_ObjectId", m_Id);

				Renderer3D::DrawQuad();
				break;
			}

			default:
				break;
			}
		}

		return false;
	}
}