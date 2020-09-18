#include "lppch.h"
#include "Entity.h"

#include "EntityManager.h"
#include "Lamp/Objects/ObjectLayer.h"

namespace Lamp
{
	void Entity::OnEvent(Event& e)
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

	void Entity::Destroy()
	{
		EntityManager::Get()->Remove(this);
		ObjectLayerManager::Get()->Remove(this);

		delete this;
	}

	Entity* Entity::Create()
	{
		return EntityManager::Get()->Create();
	}
}