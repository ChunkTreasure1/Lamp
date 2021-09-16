#include "lppch.h"
#include "Entity.h"

#include "Lamp/Level/Level.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/AssetSystem/ResourceCache.h"

#include <glm/ext/matrix_transform.hpp>
#include "Lamp/GraphKey/GraphKeyGraph.h"
#include "ComponentRegistry.h"
#include "Lamp/Objects/Entity/BaseComponents/Physics/BoxColliderComponent.h"
#include "Lamp/Meshes/Materials/MaterialLibrary.h"

namespace Lamp
{
	Entity::Entity()
	{
		m_Name = "Entity";

		m_GizmoMaterial = MaterialLibrary::GetMaterial("gizmo");
	}

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

	Entity* Entity::Create(bool saveable, uint32_t layer)
	{
		Entity* pEnt = new Entity();
		pEnt->SetLayerID(layer);
		pEnt->SetSaveable(saveable);

		g_pEnv->pLevel->GetEntities().emplace(std::make_pair(pEnt->GetID(), pEnt));
		g_pEnv->pLevel->AddToLayer(pEnt);

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
			Ref<EntityComponent> component = ComponentRegistry::Create(comp->GetName());

			for (auto& property : comp->GetComponentProperties().GetProperties())
			{
				switch (property.PropertyType)
				{
					case Lamp::PropertyType::String:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.Name == property.Name)
							{
								std::string* p = static_cast<std::string*>(prop.Value);
								*p = std::string(*static_cast<std::string*>(property.Value));

								EntityPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}

						break;
					}
					case Lamp::PropertyType::Bool:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.Name == property.Name)
							{
								bool* p = static_cast<bool*>(prop.Value);
								*p = *static_cast<bool*>(property.Value);

								EntityPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Int:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.Name == property.Name)
							{
								int* p = static_cast<int*>(prop.Value);
								*p = *static_cast<int*>(property.Value);

								EntityPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.Name == property.Name)
							{
								float* p = static_cast<float*>(prop.Value);
								*p = *static_cast<float*>(property.Value);

								EntityPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float2:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.Name == property.Name)
							{
								glm::vec2* p = static_cast<glm::vec2*>(prop.Value);
								*p = *static_cast<glm::vec2*>(property.Value);

								EntityPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float3:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.Name == property.Name)
							{
								glm::vec3* p = static_cast<glm::vec3*>(prop.Value);
								*p = *static_cast<glm::vec3*>(property.Value);

								EntityPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float4:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.Name == property.Name)
							{
								glm::vec4* p = static_cast<glm::vec4*>(prop.Value);
								*p = *static_cast<glm::vec4*>(property.Value);

								EntityPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Path:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.Name == property.Name)
							{
								std::string* p = static_cast<std::string*>(prop.Value);
								*p = *static_cast<std::string*>(property.Value);

								EntityPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}

						break;
					}
					case Lamp::PropertyType::Color3:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.Name == property.Name)
							{
								glm::vec3* p = static_cast<glm::vec3*>(prop.Value);
								*p = *static_cast<glm::vec3*>(property.Value);

								EntityPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Color4:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.Name == property.Name)
							{
								glm::vec4* p = static_cast<glm::vec4*>(prop.Value);
								*p = *static_cast<glm::vec4*>(property.Value);

								EntityPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
				}
			}

			copy->AddComponent(component);
		}

		if (entity->m_GraphKeyGraph)
		{
			copy->m_GraphKeyGraph = CreateRef<GraphKeyGraph>(*entity->m_GraphKeyGraph);
		}
		copy->m_ModelMatrix = entity->m_ModelMatrix;
		copy->SetPosition(entity->m_Position);
		copy->SetRotation(entity->m_Rotation);
		copy->SetScale(entity->m_Scale);
		copy->m_LayerID = entity->m_LayerID;


		if (addToLevel)
		{
			g_pEnv->pLevel->GetEntities().emplace(copy->m_Id, copy);
			g_pEnv->pLevel->AddToLayer(copy);
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
			glm::vec3 dir = glm::normalize(e.GetCamera()->GetPosition() - m_Position);

			float angleXZ = std::atan2f(dir.z, dir.x);
			float angleY = -std::asin(dir.y);

			glm::mat4 rotation = glm::rotate(glm::mat4(1.f), -angleXZ + glm::radians(90.f), { 0.f, 1.f, 0.f })
				* glm::rotate(glm::mat4(1.f), angleY, { 1.f, 0.f, 0.f });

			glm::mat4 transform = glm::translate(glm::mat4(1.f), m_Position)
				* rotation
				* glm::scale(glm::mat4(1.f), glm::vec3(0.5f));


			Renderer3D::SubmitQuadForward(transform, m_GizmoMaterial, m_Id);

				//switch (e.GetPassInfo().type)
				//{
				//case PassType::Forward:
				//{
				//	m_GizmoShader->Bind();

				//	m_GizmoShader->UploadMat4("u_ViewProjection", e.GetCamera()->GetViewProjectionMatrix());


				//	m_GizmoShader->UploadMat4("u_Model", model);
				//	m_GizmoShader->UploadInt("u_Texture", 0);

				//	m_GizmoTexure->Bind(0);

				//	Renderer3D::DrawQuad();
				//	break;
				//}

				//case PassType::Selection:
				//{
				//	m_SelectionShader->Bind();

				//	m_SelectionShader->UploadMat4("u_ViewProjection", e.GetCamera()->GetViewProjectionMatrix());

				//	glm::vec3 dir = glm::normalize(e.GetCamera()->GetPosition() - m_Position);

				//	float angleXZ = std::atan2f(dir.z, dir.x);
				//	float angleY = -std::asin(dir.y);

				//	glm::mat4 rotation = glm::rotate(glm::mat4(1.f), -angleXZ + glm::radians(90.f), { 0.f, 1.f, 0.f })
				//		* glm::rotate(glm::mat4(1.f), angleY, { 1.f, 0.f, 0.f });

				//	glm::mat4 model = glm::translate(glm::mat4(1.f), m_Position)
				//		* rotation
				//		* glm::scale(glm::mat4(1.f), glm::vec3(0.5f));
				//	m_SelectionShader->UploadMat4("u_Model", model);
				//	m_SelectionShader->UploadInt("u_ObjectId", m_Id);

				//	Renderer3D::DrawQuad();
				//	break;
				//}

				//default:
				//	break;
				//}
		}

		return false;
	}
}