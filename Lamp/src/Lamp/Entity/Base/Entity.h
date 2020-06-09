#pragma once

#include <vector>
#include <memory>

#include "Lamp/Entity/Base/BaseComponent.h"
#include <glm/glm.hpp>
#include "Lamp/Event/Event.h"

#include <algorithm>
#include "Physical/PhysicalEntity.h"
#include <glm/gtc/matrix_transform.hpp>

class EntityManager;

namespace Lamp
{
	class Entity
	{
	public:
		Entity()
			: m_Position(0, 0, 0), m_Rotation(0, 0, 0), m_Scale(1, 1, 1), m_Name("")
		{
			m_pPhysicalEntity = std::make_shared<PhysicalEntity>();
			m_pPhysicalEntity->SetCollider(std::make_shared<BoundingSphere>(m_Position, 1.f));
		}
		~Entity() {}

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
		inline std::vector<Ref<EntityComponent>> GetComponents() const { return m_pComponents; }
		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::vec3& GetRotation() const { return m_Rotation; }
		inline const glm::vec3& GetScale() const { return m_Scale; }

		inline const std::string& GetName() const { return m_Name; }
		inline glm::mat4& GetModelMatrix() { return m_ModelMatrix; }
		inline Ref<PhysicalEntity>& GetPhysicalEntity() { return m_pPhysicalEntity; }

		//Setting
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; CalculateModelMatrix(); }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; CalculateModelMatrix(); }
		inline void SetScale(const glm::vec3& scale) { m_Scale = scale; CalculateModelMatrix(); }

		inline void SetName(const std::string& name) { m_Name = name; }
		inline void SetModelMatrix(const glm::mat4& mat) { m_ModelMatrix = mat; }

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

		bool AddComponent(Ref<EntityComponent> comp)
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
		void CalculateModelMatrix()
		{
			m_ModelMatrix = glm::translate(glm::mat4(1.f), m_Position)
				* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.x + 90.f), glm::vec3(1.f, 0.f, 0.f))
				* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.y), glm::vec3(0.f, 1.f, 0.f))
				* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.z), glm::vec3(0.f, 0.f, 1.f))
				* glm::scale(glm::mat4(1.f), m_Scale);
		}

	private:
		bool m_IsActive = true;
		Ref<PhysicalEntity> m_pPhysicalEntity;

		std::vector<Ref<EntityComponent>> m_pComponents;
		std::unordered_map<std::string, Ref<EntityComponent>> m_pComponentMap;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;

		glm::mat4 m_ModelMatrix;
		std::string m_Name;
	};
}