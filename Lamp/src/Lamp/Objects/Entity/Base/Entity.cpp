#include "lppch.h"
#include "Entity.h"

#include "EntityManager.h"
#include "Lamp/Objects/ObjectLayer.h"

namespace Lamp
{
	void Entity::OnEvent(Event& e)
	{
		for (auto it = m_pComponents.begin(); it != m_pComponents.end(); it++)
		{
			if (m_pComponents.size() == 0)
			{
				return;
			}

			if (it->get()->GetEventMask() & e.GetEventType())
			{
				it->get()->OnEvent(e);
			}
		}
	}

	void Entity::Destroy()
	{
		EntityManager::Get()->Remove(this);
		ObjectLayerManager::Get()->Remove(this);

		delete this;
	}

	Entity* Entity::Create()
	{
		return EntityManager::Get()->Create(false);
	}
}