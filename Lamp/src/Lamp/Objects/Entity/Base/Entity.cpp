#include "lppch.h"
#include "Entity.h"

#include "Lamp/Level/Level.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"

#include <glm/ext/matrix_transform.hpp>
#include "Lamp/GraphKey/GraphKeyGraph.h"
#include "ComponentRegistry.h"

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
		auto& entites = g_pEnv->pLevel->GetEntities();
		entites.erase(m_Id);
		delete this;
	}

	Entity* Entity::Create(bool saveable)
	{
		Entity* pEnt = new Entity();
		pEnt->SetLayerID(0);
		pEnt->SetSaveable(saveable);

		g_pEnv->pLevel->GetEntities().emplace(std::make_pair(pEnt->GetID(), pEnt));

		return pEnt;
	}

	Entity* Entity::Get(uint32_t id)
	{
		if (auto it = g_pEnv->pLevel->GetEntities().find(id); it != g_pEnv->pLevel->GetEntities().end())
		{
			return g_pEnv->pLevel->GetEntities().at(id);
		}

		return nullptr;
	}

	Entity* Entity::Duplicate(Entity* entity, bool addToLevel)
	{
		Entity* copy = new Entity();
		for (auto& comp : entity->GetComponents())
		{
			copy->AddComponent(ComponentRegistry::Create(comp->GetName()));
		}

		if (entity->m_GraphKeyGraph)
		{
			copy->m_GraphKeyGraph = CreateRef<GraphKeyGraph>(*entity->m_GraphKeyGraph);
		}
		copy->m_ModelMatrix = entity->m_ModelMatrix;
		copy->m_Position = entity->m_Position;
		copy->m_Rotation = entity->m_Rotation;
		copy->m_Scale = entity->m_Scale;

		if (addToLevel)
		{
			g_pEnv->pLevel->GetEntities().emplace(copy->m_Id, copy);
		}
		else
		{
			copy->m_Id = entity->m_Id;
		}

		return copy;
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

				m_GizmoShader->UploadMat4("u_ViewProjection", e.GetCamera()->GetViewProjectionMatrix());

				glm::vec3 dir = glm::normalize(e.GetCamera()->GetPosition() - m_Position);
				
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

				m_SelectionShader->UploadMat4("u_ViewProjection", e.GetCamera()->GetViewProjectionMatrix());

				glm::vec3 dir = glm::normalize(e.GetCamera()->GetPosition() - m_Position);

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