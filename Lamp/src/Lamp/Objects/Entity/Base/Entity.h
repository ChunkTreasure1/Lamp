#pragma once

#include <vector>
#include <memory>

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include <glm/glm.hpp>
#include "Lamp/Event/Event.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "Lamp/Objects/Object.h"
#include <string>

#include "Lamp/Event/ApplicationEvent.h"
namespace Lamp
{
	class GraphKeyGraph;
	class EntityManager;

	class Entity : public Object
	{
	public:
		Entity();
		~Entity() = default;

		virtual void OnEvent(Event& e) override;
		virtual void Destroy() override;

		inline void SetSaveable(bool state) { m_shouldBeSaved = state; }
		inline const bool GetSaveable() const { return m_shouldBeSaved; }

		inline void SetGraphKeyGraph(Ref<GraphKeyGraph> graph) { m_graphKeyGraph = graph; }
		inline const Ref<GraphKeyGraph> GetGraphKeyGraph() const { return m_graphKeyGraph; }

		inline void SetGizmoMaterial(Ref<Material> mat) { m_gizmoMaterial = mat; }
		inline const Ref<Material> GetGizmoMaterial() { return m_gizmoMaterial; }

		//Getting
		inline const std::vector<Ref<EntityComponent>>& GetComponents() const { return m_pComponents; }

		template<typename T>
		Ref<T> GetComponent()
		{
			if (auto it = m_pComponentMap.find(T::GetFactoryName()); it != m_pComponentMap.end())
			{
				return std::dynamic_pointer_cast<T>(it->second);
			}

			return Ref<T>(nullptr);
		}

		template<typename T, typename... TArgs>
		Ref<T> GetOrCreateComponent(TArgs&&... mArgs)
		{
			if (auto it = m_pComponentMap.find(T::GetFactoryName()); it == m_pComponentMap.end())
			{
				Ref<T> c = CreateRef<T>(std::forward<TArgs>(mArgs)...);
				c->m_pEntity = this;

				m_pComponents.push_back(c);
				m_pComponentMap[T::GetFactoryName()] = c;

				c->Initialize();

				return c;
			}
			else
			{
				return std::dynamic_pointer_cast<T>(it->second);
			}
		}
		

		template<typename T>
		bool HasComponent()
		{
			if (auto it = m_pComponentMap.find(T::GetFactoryName()); it != m_pComponentMap.end())
			{
				return true;
			}

			return false;
		}

		bool HasComponent(const std::string& name)
		{
			if (auto it = m_pComponentMap.find(name); it != m_pComponentMap.end())
			{
				return true;
			}

			return false;
		}

		bool AddComponent(Ref<EntityComponent> comp)
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

		bool RemoveComponent(Ref<EntityComponent> comp)
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

		template<typename T>
		bool RemoveComponent()
		{
			for (auto it = m_pComponents.begin(); it != m_pComponents.end(); it++)
			{
				if (it->get()->GetName() == T::GetFactoryName())
				{
					m_pComponents.erase(it);

					if (auto t = m_pComponentMap.find(T::GetFactoryName()); t != m_pComponentMap.end())
					{
						m_pComponentMap.erase(t);
						return true;
					}

					return true;
				}
			}

			return false;
		}

	public:
		static Entity* Create(bool saveable = false, uint32_t layer = 0);
		static Entity* Get(uint32_t id);
		static Entity* Duplicate(Entity* entity, bool addToLevel = true);

	private:
		bool OnRenderEvent(AppRenderEvent& e);

	private:
		bool m_shouldBeSaved = false;

		Ref<GraphKeyGraph> m_graphKeyGraph = nullptr;
		Ref<Material> m_gizmoMaterial;

		std::vector<Ref<EntityComponent>> m_pComponents;
		std::unordered_map<std::string, Ref<EntityComponent>> m_pComponentMap;
	};
}