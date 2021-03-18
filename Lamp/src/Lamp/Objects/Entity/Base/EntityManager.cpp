#include "lppch.h"
#include "EntityManager.h"

#include "Lamp/Physics/PhysicsEngine.h"
#include "Lamp/Objects/ObjectLayer.h"

namespace Lamp
{
	EntityManager::EntityManager()
	{
	}

	EntityManager::~EntityManager()
	{
		m_pEntities.clear();
	}

	Entity* EntityManager::Create(bool saveable)
	{
		Entity* pEnt = new Entity();
		pEnt->SetLayerID(0);
		pEnt->SetSaveable(saveable);

		m_pEntities.emplace(std::make_pair(pEnt->GetID(), pEnt));

		ObjectLayerManager::Get()->AddToLayer(pEnt, 0);

		return pEnt;
	}

	bool EntityManager::Remove(Entity* pEnt)
	{
		if (m_pEntities.size() == 0 || !pEnt)
		{
			return false;
		}

		m_pEntities.erase(pEnt->GetID());

		return true;
	}

	Entity* EntityManager::GetEntityFromId(uint32_t id)
	{
		return m_pEntities[id];
	}

	Entity* EntityManager::GetEntityFromPoint(const glm::vec3& pos, const glm::vec3& origin)
	{
		//Entity* entity = dynamic_cast<Entity*>(ObjectLayerManager::Get()->GetObjectFromPoint(pos, origin));
		//if (entity)
		//{
		//	return entity;
		//}

		return nullptr;
	}

}