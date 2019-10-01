#pragma once

#include <vector>

#include "LampEntity/EntityComponent/EntityComponent.h"

namespace LampEntity
{
	class IEntity
	{
	public:
		IEntity() {}
		~IEntity() {}
		
		void Update() {}
		void Draw() {}

		template<typename T>
		T* GetComponent()
		{
			if (m_ComponentBitSet[GetComponentTypeID<T>()])
			{
				return (T*)m_ComponentArray[GetComponentTypeID<T>()];
			}
			else
			{
				return nullptr;
			}
		}

		template<class T>
		T* GetOrCreateComponent()
		{
			if (!m_ComponentBitSet[GetComponentTypeID<T>()])
			{
				T* c(new T());
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