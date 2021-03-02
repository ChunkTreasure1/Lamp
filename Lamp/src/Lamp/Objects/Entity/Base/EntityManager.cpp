#include "lppch.h"
#include "EntityManager.h"

#include "Lamp/Physics/PhysicsEngine.h"
#include "Lamp/Objects/ObjectLayer.h"

namespace Lamp
{
	EntityManager* EntityManager::s_CurrentManager = nullptr;

	EntityManager::EntityManager()
	{
		s_CurrentManager = this;
	}

	EntityManager::~EntityManager()
	{
		m_pEntites.clear();
	}

	Entity* EntityManager::Create(bool saveable)
	{
		Entity* pEnt = new Entity();
		pEnt->SetLayerID(0);
		pEnt->SetSaveable(saveable);

		m_pEntites.emplace_back(pEnt);

		ObjectLayerManager::Get()->AddToLayer(pEnt, 0);

		return pEnt;
	}

	bool EntityManager::Remove(Entity* pEnt)
	{
		if (m_pEntites.size() == 0)
		{
			return false;
		}

		auto it = std::find(m_pEntites.begin(), m_pEntites.end(), pEnt);
		if (it != m_pEntites.end())
		{
			m_pEntites.erase(it);
		}
		else
		{
			return false;
		}

		return true;
	}

	Entity* EntityManager::GetEntityFromId(uint32_t id)
	{
		Entity* ent = nullptr;

		auto func = [&id, &ent](Entity* pEnt)
		{
			if (pEnt->GetID() == id)
			{
				ent = pEnt;
			}
		};

		std::for_each(std::execution::par_unseq, m_pEntites.begin(), m_pEntites.end(), func);

		return ent;
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