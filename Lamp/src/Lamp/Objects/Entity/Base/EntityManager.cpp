#include "lppch.h"
#include "EntityManager.h"

#include "Lamp/Physics/PhysicsEngine.h"
#include "Lamp/Objects/ObjectLayer.h"

namespace Lamp
{
	Ref<EntityManager> EntityManager::s_CurrentManager;

	void EntityManager::Shutdown()
	{
		for (size_t i = 0; i < m_pEntites.size(); i++)
		{
			delete m_pEntites[i];
		}

		m_pEntites.clear();
	}

	Entity* EntityManager::Create()
	{
		Entity* pEnt = new Entity();
		pEnt->SetLayerID(0);

		m_pEntites.emplace_back(pEnt);

		ObjectLayerManager::Get()->AddToLayer(pEnt, 0);
		PhysicsEngine::Get()->AddEntity(pEnt->GetPhysicalEntity());

		return pEnt;
	}

	bool EntityManager::Remove(Entity* pEnt)
	{
		pEnt->Destroy();
		auto it = std::find(m_pEntites.begin(), m_pEntites.end(), pEnt);
		if (it != m_pEntites.end())
		{
			m_pEntites.erase(it);
		}
		else
		{
			return false;
		}

		ObjectLayerManager::Get()->RemoveFromLayer(dynamic_cast<Object*>(pEnt), pEnt->GetLayerID());
		PhysicsEngine::Get()->RemoveEntity(pEnt->GetPhysicalEntity());
		return true;
	}

	Entity* EntityManager::GetEntityFromPhysicalEntity(PhysicalEntity* pEnt)
	{
		for (auto& ent : m_pEntites)
		{
			if (ent->GetPhysicalEntity().get() == pEnt)
			{
				return ent;
			}
		}

		return nullptr;
	}

	Entity* EntityManager::GetEntityFromPoint(const glm::vec3& pos, const glm::vec3& origin)
	{
		Entity* entity = dynamic_cast<Entity*>(ObjectLayerManager::Get()->GetObjectFromPoint(pos, origin));
		if (entity)
		{
			return entity;
		}

		return nullptr;
	}
}