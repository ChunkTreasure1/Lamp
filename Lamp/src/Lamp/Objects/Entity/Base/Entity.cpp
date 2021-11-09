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
				switch (property.propertyType)
				{
					case Lamp::PropertyType::String:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.name == property.name)
							{
								std::string* p = static_cast<std::string*>(prop.value);
								*p = std::string(*static_cast<std::string*>(property.value));

								ObjectPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}

						break;
					}
					case Lamp::PropertyType::Bool:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.name == property.name)
							{
								bool* p = static_cast<bool*>(prop.value);
								*p = *static_cast<bool*>(property.value);

								ObjectPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Int:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.name == property.name)
							{
								int* p = static_cast<int*>(prop.value);
								*p = *static_cast<int*>(property.value);

								ObjectPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.name == property.name)
							{
								float* p = static_cast<float*>(prop.value);
								*p = *static_cast<float*>(property.value);

								ObjectPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float2:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.name == property.name)
							{
								glm::vec2* p = static_cast<glm::vec2*>(prop.value);
								*p = *static_cast<glm::vec2*>(property.value);

								ObjectPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float3:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.name == property.name)
							{
								glm::vec3* p = static_cast<glm::vec3*>(prop.value);
								*p = *static_cast<glm::vec3*>(property.value);

								ObjectPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Float4:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.name == property.name)
							{
								glm::vec4* p = static_cast<glm::vec4*>(prop.value);
								*p = *static_cast<glm::vec4*>(property.value);

								ObjectPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Path:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.name == property.name)
							{
								std::string* p = static_cast<std::string*>(prop.value);
								*p = *static_cast<std::string*>(property.value);

								ObjectPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}

						break;
					}
					case Lamp::PropertyType::Color3:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.name == property.name)
							{
								glm::vec3* p = static_cast<glm::vec3*>(prop.value);
								*p = *static_cast<glm::vec3*>(property.value);

								ObjectPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
					case Lamp::PropertyType::Color4:
					{
						for (auto& prop : component->GetComponentProperties().GetProperties())
						{
							if (prop.name == property.name)
							{
								glm::vec4* p = static_cast<glm::vec4*>(prop.value);
								*p = *static_cast<glm::vec4*>(property.value);

								ObjectPropertyChangedEvent e;
								component->OnEvent(e);
							}
						}
						break;
					}
				}
			}

			copy->AddComponent(component);
		}

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
		if (g_pEnv->ShouldRenderGizmos && m_IsActive)
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


			Renderer2D::SubmitQuad(transform, m_gizmoMaterial, m_Id);
		}

		return false;
	}
}