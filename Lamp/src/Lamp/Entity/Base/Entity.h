#pragma once

#include <vector>
#include <memory>

#include "Lamp/Entity/Base/BaseComponent.h"
#include <glm/glm.hpp>
#include "Lamp/Event/Event.h"

#include <algorithm>
#include "Physical/PhysicalEntity.h"

class EntityManager;

namespace Lamp
{
	class IEntity
	{
	public:
		IEntity()
			: m_Position(0, 0, 0), m_Rotation(0, 0, 0), m_Scale(1, 1, 1), m_Name("")
		{
			m_pPhysicalEntity = std::make_shared<PhysicalEntity>();
			m_pPhysicalEntity->SetEntity(this);
			m_pPhysicalEntity->SetCollider(std::make_shared<BoundingSphere>(m_Position, 1.f));
		}
		~IEntity() {}

		void Update(Timestep ts)
		{
			for (auto& pComp : m_pComponents)
			{
				if (m_pComponents.size() == 0)
				{
					return;
				}

				pComp->Update(ts);
			}
		}
		void Draw()
		{
			for (auto& pComp : m_pComponents)
			{
				if (m_pComponents.size() == 0)
				{
					return;
				}

				pComp->Draw();
			}
		}
		void OnEvent(Event& e)
		{
			for (auto& pComp : m_pComponents)
			{
				if (m_pComponents.size() == 0)
				{
					return;
				}

				pComp->OnEvent(e);
			}
		}
		void Destroy()
		{
		}

		//Getting
		inline std::vector<Ref<IEntityComponent>> GetComponents() const { return m_pComponents; }
		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::vec3& GetRotation() const { return m_Rotation; }
		inline const glm::vec3& GetScale() const { return m_Scale; }

		inline const std::string& GetName() const { return m_Name; }
		inline Ref<PhysicalEntity>& GetPhysicalEntity() { return m_pPhysicalEntity; }

		//Setting
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; }
		inline void SetScale(const glm::vec3& scale) { m_Scale = scale; }

		inline void SetName(const std::string& name) { m_Name = name; }

		template<typename T>
		Ref<T> GetComponent()
		{
			if (auto it = m_pComponentMap.find(T::GetFactoryName()); it != m_pComponentMap.end())
			{
				return std::dynamic_pointer_cast<T>(it->second);
			}

			return std::shared_ptr<T>(nullptr);
		}

		template<typename T, typename... TArgs>
		Ref<T> GetOrCreateComponent(TArgs&&... mArgs)
		{
			if (auto it = m_pComponentMap.find(T::GetFactoryName()); it == m_pComponentMap.end())
			{
				Ref<T> c(new T(std::forward<TArgs>(mArgs)...));
				c->MakeOwner(this);

				m_pComponents.emplace_back(c);

				m_pComponentMap[T::GetFactoryName()] = c;

				c->Initialize();

				return c;
			}
			else
			{
				return std::dynamic_pointer_cast<T>(it->second);
			}
		}

		bool AddComponent(Ref<IEntityComponent> comp)
		{
			std::string str = comp->GetName();
			str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());

			if (auto it = m_pComponentMap.find(str); it == m_pComponentMap.end())
			{
				m_pComponentMap[str] = std::move(comp);
				return true;
			}

			return false;
		}

	private:
		bool m_IsActive = true;
		Ref<PhysicalEntity> m_pPhysicalEntity;

		std::vector<Ref<IEntityComponent>> m_pComponents;
		std::unordered_map<std::string, Ref<IEntityComponent>> m_pComponentMap;

		ComponentArray m_pComponentArray;
		ComponentBitSet m_ComponentBitSet;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;

		std::string m_Name;
	};
}