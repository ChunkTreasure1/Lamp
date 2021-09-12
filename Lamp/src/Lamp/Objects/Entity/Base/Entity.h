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
		Entity()
		{
			m_Name = "Entity" + std::to_string(m_Id);

			m_GizmoTexure = Texture2D::Create("engine/gizmos/gizmoEntity.png");
		}
		~Entity() {}

		virtual void OnEvent(Event& e) override;
		virtual void Destroy() override;
		virtual uint64_t GetEventMask() override { return EventType::All; }

		inline void SetSaveable(bool state) { m_ShouldBeSaved = state; }
		inline bool GetSaveable() { return m_ShouldBeSaved; }
		inline uint32_t GetId() { return m_Id; }
		inline void SetGraphKeyGraph(Ref<GraphKeyGraph> graph) { m_GraphKeyGraph = graph; }
		inline Ref<GraphKeyGraph>& GetGraphKeyGraph() { return m_GraphKeyGraph; }

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
				Ref<T> c(new T(std::forward<TArgs>(mArgs)...));
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
		static Entity* Create(bool saveable = false);
		static Entity* Get(uint32_t id);
		static Entity* Duplicate(Entity* entity, bool addToLevel = true);

	private:
		bool OnRenderEvent(AppRenderEvent& e);

	private:
		bool m_ShouldBeSaved = false;

		Ref<Texture2D> m_GizmoTexure = nullptr;
		Ref<Shader> m_GizmoShader = nullptr;
		Ref<Shader> m_SelectionShader = nullptr;
		Ref<GraphKeyGraph> m_GraphKeyGraph = nullptr;

		std::vector<Ref<EntityComponent>> m_pComponents;
		std::unordered_map<std::string, Ref<EntityComponent>> m_pComponentMap;
	};
}