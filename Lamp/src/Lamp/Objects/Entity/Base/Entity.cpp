#include "lppch.h"
#include "Entity.h"

#include "Lamp/Level/Level.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/AssetSystem/ResourceCache.h"

#include <glm/ext/matrix_transform.hpp>
#include "Lamp/GraphKey/GraphKeyGraph.h"
#include "ComponentRegistry.h"
#include "Lamp/Objects/Entity/BaseComponents/Physics/BoxColliderComponent.h"
#include "Lamp/Mesh/Materials/MaterialLibrary.h"

namespace Lamp
{
	Entity::Entity()
	{
		m_Name = "Entity";
		m_gizmoMaterial = MaterialLibrary::GetMaterial("gizmoEntity");
	}

	void Entity::OnEvent(Event& e)
	{
		if (!m_IsActive)
		{
			return;
		}

		for (const auto& comp : m_pComponents)
		{
			comp->OnEvent(e);
		}

		if (m_graphKeyGraph)
		{
			m_graphKeyGraph->OnEvent(e);
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(Entity::OnRenderEvent));
	}

	void Entity::Destroy()
	{
		auto& entites = g_pEnv->pLevel->GetEntities();
		g_pEnv->pLevel->RemoveFromLayer(this);

		entites.erase(m_Id);
		delete this;
	}

	bool Entity::HasComponent(const std::string& name)
	{
		if (auto it = m_pComponentMap.find(name); it != m_pComponentMap.end())
		{
			return true;
		}

		return false;
	}

	bool Entity::AddComponent(Ref<EntityComponent> comp)
	{
		std::string str = comp->GetName();
		str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());

		if (auto it = m_pComponentMap.find(str); it == m_pComponentMap.end())
		{
			comp->m_pEntity = this;
			comp->Initialize();

			m_pComponents.push_back(comp);
			m_pComponentMap[str] = comp;
			return true;
		}

		return false;
	}

	bool Entity::RemoveComponent(Ref<EntityComponent> comp)
	{
		auto it = std::find(m_pComponents.begin(), m_pComponents.end(), comp);
		if (it != m_pComponents.end())
		{
			m_pComponents.erase(it);

			if (auto t = m_pComponentMap.find(comp->GetName()); t != m_pComponentMap.end())
			{
				m_pComponentMap.erase(t);
				return true;
			}
		}

		return false;
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

		DuplicateComponents(copy, entity);

		if (entity->m_graphKeyGraph)
		{
			copy->m_graphKeyGraph = CreateRef<GraphKeyGraph>(*entity->m_graphKeyGraph);
		}
		copy->m_Transform = entity->m_Transform;
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
		if (g_pEnv->shouldRenderGizmos && m_IsActive)
		{
			glm::vec3 dir = glm::normalize(e.GetCamera()->GetPosition() - m_Position);

			float angleXZ = std::atan2f(dir.z, dir.x);
			float angleY = -std::asin(dir.y);

			const float maxDist = 10.f;
			const float maxScale = 3.f;

			float distance = glm::distance(e.GetCamera()->GetPosition(), m_Position);
			float scale = glm::min(distance / maxDist, maxScale);

			glm::mat4 rotation = glm::rotate(glm::mat4(1.f), -angleXZ + glm::radians(90.f), { 0.f, 1.f, 0.f })
				* glm::rotate(glm::mat4(1.f), angleY, { 1.f, 0.f, 0.f });

			glm::mat4 transform = glm::translate(glm::mat4(1.f), m_Position)
				* rotation
				* glm::scale(glm::mat4(1.f), glm::vec3(scale));


			//Renderer2D::SubmitQuad(transform, m_gizmoMaterial, m_Id);
		}

		return false;
	}

	void Entity::DuplicateComponents(Entity* copy, Entity* entity)
	{
		for (auto& comp : entity->GetComponents())
		{
			Ref<EntityComponent> component = ComponentRegistry::Create(comp->GetName());

			for (auto& property : comp->GetComponentProperties().GetProperties())
			{
				bool changed = false;

				switch (property.propertyType)
				{
					case Lamp::PropertyType::String:
					{
						if (auto p = GetPropertyData<std::string>(property.name, component->GetComponentProperties().GetProperties()))
						{
							*p = *static_cast<std::string*>(property.value);
							changed = true;
						}

						break;
					}

					case Lamp::PropertyType::Bool:
					{
						if (auto p = GetPropertyData<std::string>(property.name, component->GetComponentProperties().GetProperties()))
						{
							*p = *static_cast<std::string*>(property.value);
							changed = true;
						}

						break;
					}

					case Lamp::PropertyType::Int:
					{
						if (auto p = GetPropertyData<std::string>(property.name, component->GetComponentProperties().GetProperties()))
						{
							*p = *static_cast<std::string*>(property.value);
							changed = true;
						}

						break;
					}

					case Lamp::PropertyType::Float:
					{
						if (auto p = GetPropertyData<std::string>(property.name, component->GetComponentProperties().GetProperties()))
						{
							*p = *static_cast<std::string*>(property.value);
							changed = true;
						}

						break;
					}

					case Lamp::PropertyType::Float2:
					{
						if (auto p = GetPropertyData<std::string>(property.name, component->GetComponentProperties().GetProperties()))
						{
							*p = *static_cast<std::string*>(property.value);
							changed = true;
						}

						break;
					}

					case Lamp::PropertyType::Float3:
					{
						if (auto p = GetPropertyData<std::string>(property.name, component->GetComponentProperties().GetProperties()))
						{
							*p = *static_cast<std::string*>(property.value);
							changed = true;
						}

						break;
					}

					case Lamp::PropertyType::Float4:
					{
						if (auto p = GetPropertyData<std::string>(property.name, component->GetComponentProperties().GetProperties()))
						{
							*p = *static_cast<std::string*>(property.value);
							changed = true;
						}

						break;
					}

					case Lamp::PropertyType::Path:
					{
						if (auto p = GetPropertyData<std::string>(property.name, component->GetComponentProperties().GetProperties()))
						{
							*p = *static_cast<std::string*>(property.value);
							changed = true;
						}

						break;
					}

					case Lamp::PropertyType::Color3:
					{
						if (auto p = GetPropertyData<std::string>(property.name, component->GetComponentProperties().GetProperties()))
						{
							*p = *static_cast<std::string*>(property.value);
							changed = true;
						}

						break;
					}

					case Lamp::PropertyType::Color4:
					{
						if (auto p = GetPropertyData<std::string>(property.name, component->GetComponentProperties().GetProperties()))
						{
							*p = *static_cast<std::string*>(property.value);
							changed = true;
						}

						break;
					}
				}

				if (changed)
				{
					ObjectPropertyChangedEvent e;
					component->OnEvent(e);
				}
			}

			copy->AddComponent(component);
		}
	}
}