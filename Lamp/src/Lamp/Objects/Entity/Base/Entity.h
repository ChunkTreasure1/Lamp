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
	class Material;

	class Entity : public Object
	{
	public:
		Entity();
		~Entity() override = default;

		void OnEvent(Event& e) override;
		void Destroy() override;

		inline void SetSaveable(bool state) { m_shouldBeSaved = state; }
		inline void SetGraphKeyGraph(Ref<GraphKeyGraph> graph) { m_graphKeyGraph = graph; }
		inline void SetGizmoMaterial(Ref<Material> mat) { m_gizmoMaterial = mat; }

		inline const bool GetSaveable() const { return m_shouldBeSaved; }
		inline const Ref<GraphKeyGraph> GetGraphKeyGraph() const { return m_graphKeyGraph; }
		inline const Ref<Material> GetGizmoMaterial() { return m_gizmoMaterial; }
		inline const std::vector<Ref<EntityComponent>>& GetComponents() const { return m_pComponents; }

		template<typename T> Ref<T> GetComponent();
		template<typename T, typename... TArgs> Ref<T> GetOrCreateComponent(TArgs&&... mArgs);
		template<typename T> bool HasComponent();
		template<typename T> bool RemoveComponent();

		bool HasComponent(const std::string& name);
		bool AddComponent(Ref<EntityComponent> comp);
		bool RemoveComponent(Ref<EntityComponent> comp);

		static Entity* Create(bool saveable = false, uint32_t layer = 0, bool addToLevel = true);
		static Entity* Get(uint32_t id);
		static Entity* Duplicate(Entity* entity, bool addToLevel = true);

	private:
		bool OnRenderEvent(AppRenderEvent& e);

		template<typename T> 
		static T* GetPropertyData(const std::string& name, const std::vector<ComponentProperty>& properties)
		{
			for (const auto& prop : properties)
			{
				if (prop.name == name)
				{
					T* p = static_cast<T*>(prop.value);
					return p;
				}
			}

			return nullptr;
		}

		static void DuplicateComponents(Entity* copy, Entity* entity);

		bool m_shouldBeSaved = false;

		Ref<GraphKeyGraph> m_graphKeyGraph = nullptr;
		Ref<Material> m_gizmoMaterial;

		std::vector<Ref<EntityComponent>> m_pComponents;
		std::unordered_map<std::string, Ref<EntityComponent>> m_pComponentMap;
	};
	
	template<typename T>
	inline Ref<T> Entity::GetComponent()
	{
		if (auto it = m_pComponentMap.find(T::GetFactoryName()); it != m_pComponentMap.end())
		{
			return std::dynamic_pointer_cast<T>(it->second);
		}

		return Ref<T>(nullptr);
	}

	template<typename T, typename ...TArgs>
	inline Ref<T> Entity::GetOrCreateComponent(TArgs && ...mArgs)
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
	inline bool Entity::HasComponent()
	{
		if (auto it = m_pComponentMap.find(T::GetFactoryName()); it != m_pComponentMap.end())
		{
			return true;
		}

		return false;
	}

	template<typename T>
	inline bool Entity::RemoveComponent()
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
}