#include "Entity.h"

namespace LampEntity
{
	void Entity::Update(Lamp::Timestep ts)
	{
		for (auto& comp : m_pComponents)
		{
			comp->Update(ts);
		}
	}
	
	template<typename T>
	inline T* Entity::GetComponent()
	{
		if (m_ComponentBitSet[GetComponentTypeID<T>])
		{
			return m_ComponentArray[GetComponentTypeID<T>];
		}
		else
		{
			return nullptr;
		}
	}

	template<typename T, typename... TArgs>
	T & Entity::AddComponent(TArgs&&... mArgs)
	{
		T* c(new T(std::forward<TArgs>(mArgs)...));
		c->MakeOwner(this);
		std::unique_ptr<IEntityComponent> uPtr = std::make_unique(c);
		m_pComponents.emplace_back(std::move(uPtr));

		m_ComponentArray[GetComponentTypeID<T>()] = c;
		m_ComponentBitSet[GetComponentTypeID<T>()] = true;

		c->Initialize();

		return *C;
	}
}