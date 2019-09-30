#include "Entity.h"

namespace LampEntity
{
	void IEntity::Update()
	{
		for (auto& comp : m_pComponents)
		{
			comp->Update();
		}
	}
	
	template<typename T>
	inline T* IEntity::GetComponent()
	{
		if (m_ComponentBitSet[GetComponentTypeID()<T>])
		{
			return m_ComponentArray[GetComponentTypeID()<T>];
		}
		else
		{
			return nullptr;
		}
	}

	template<typename T, typename... TArgs>
	T * IEntity::GetOrCreateComponent(TArgs&&... mArgs)
	{
		if (!m_ComponentBitSet[GetComponentTypeID()<T>])
		{
			T* c(new T(std::forward<TArgs>(mArgs)...));
			c->MakeOwner(this);
			std::unique_ptr<IEntityComponent> uPtr = std::make_unique(c);
			m_pComponents.emplace_back(std::move(uPtr));

			m_ComponentArray[GetComponentTypeID<T>()] = c;
			m_ComponentBitSet[GetComponentTypeID<T>()] = true;

			c->Initialize();

			return C;
		}
		else
		{
			return m_ComponentArray[GetComponentTypeID() < T > ];
		}
	}

	template<typename T>
	bool IEntity::RemoveComponent()
	{
		if (m_ComponentBitSet[GetComponentTypeID()<T>])
		{
			for (size_t i = 0; i < m_pComponents.size(); i++)
			{
				if (m_pComponents[i] == m_ComponentArray[GetComponentTypeID()<T>])
				{
					m_pComponents.erase(m_pComponents.begin() + i);
					break;
				}
			}

			m_ComponentArray[GetComponentTypeID() < T > ] = nullptr;
			m_ComponentBitSet[GetComponentTypeID() < T > ] = false;
		}
		else
		{
			return false;
		}
	}
}