#pragma once

#include <vector>
#include <memory>

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include <glm/glm.hpp>
#include "Lamp/Event/Event.h"

#include <algorithm>
#include "Physical/PhysicalEntity.h"
#include <glm/gtc/matrix_transform.hpp>

#include "Lamp/Objects/Object.h"

class EntityManager;

namespace Lamp
{
	class Entity : public Object
	{
	public:
		Entity()
		{
			m_PhysicalEntity = CreateRef<PhysicalEntity>();
			m_PhysicalEntity->SetCollider(CreateRef<BoundingSphere>(m_Position, 1.f));
			m_Name = "Entity";
		}
		~Entity() {}

		virtual void OnEvent(Event& e) override;
		virtual void Destroy() override;

		inline void SetSaveable(bool state) { m_ShouldBeSaved = state; }
		inline bool GetSaveable() { return m_ShouldBeSaved; }

		//Getting
		inline std::vector<Ref<EntityComponent>> GetComponents() const { return m_pComponents; }

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
				c->MakeOwner(this);

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
				comp->MakeOwner(this);
				comp->Initialize();

				m_pComponents.push_back(comp);
				m_pComponentMap[str] = comp;
				return true;
			}

			return false;
		}

	public:
		static Entity* Create();

	private:
		bool m_ShouldBeSaved = false;

		std::vector<Ref<EntityComponent>> m_pComponents;
		std::unordered_map<std::string, Ref<EntityComponent>> m_pComponentMap;
	};
}