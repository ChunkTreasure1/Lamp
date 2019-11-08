#pragma once

#include <vector>
#include <memory>

#include "Lamp/Entity/Base/Component.h"
#include <glm/glm.hpp>

class EntityManager;

namespace Lamp
{
	class IEntity
	{
	public:
		IEntity() {}
		~IEntity() {}

		void Update() 
		{
			for (auto& pComp : m_pComponents)
			{
				pComp->Update();
			}
		}
		void Draw() 
		{
			for (auto& pComp : m_pComponents)
			{
				pComp->Draw();
			}
		}
		void Destroy()
		{
			for (size_t i = 0; i < m_pComponents.size(); i++)
			{
				delete m_pComponents[i];
			}

			m_pComponents.clear();
		}

		//Getting
		inline std::vector<IEntityComponent*> GetComponents() const { return m_pComponents; }

		template<typename T>
		const T* GetComponent()
		{
			if (m_ComponentBitSet[GetComponentTypeID<T>()])
			{
				return (T*)m_pComponentArray[GetComponentTypeID<T>()];
			}
			else
			{
				return nullptr;
			}
		}

		template<typename T, typename... TArgs>
		T* GetOrCreateComponent(TArgs&&... mArgs)
		{
			if (!m_ComponentBitSet[GetComponentTypeID<T>()])
			{
				T* c(new T(std::forward<TArgs>(mArgs)...));
				c->MakeOwner(this);

				m_pComponents.emplace_back(c);

				m_pComponentArray[GetComponentTypeID<T>()] = c;
				m_ComponentBitSet[GetComponentTypeID<T>()] = true;

				c->Initialize();

				return c;
			}
			else
			{
				return static_cast<T*>(m_pComponentArray[GetComponentTypeID<T>()]);
			}
		}

	private:
		bool m_IsActive = true;

		std::vector<IEntityComponent*> m_pComponents;
		ComponentArray m_pComponentArray;
		ComponentBitSet m_ComponentBitSet;
	};
}