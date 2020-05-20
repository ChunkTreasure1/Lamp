#include "lppch.h"
#include "EntityManager.h"

#include "Lamp/Physics/PhysicsEngine.h"

namespace Lamp
{
	Ref<EntityManager> EntityManager::s_CurrentManager;

	void EntityManager::OnEvent(Event& e)
	{
		for (Entity* pE : m_pEntites)
		{
			pE->OnEvent(e);
		}
	}

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
		m_pEntites.emplace_back(pEnt);
		
		Lamp::PhysicsEngine::Get()->AddEntity(pEnt->GetPhysicalEntity());

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
		Lamp::PhysicsEngine::Get()->RemoveEntity(pEnt->GetPhysicalEntity());

		delete pEnt;

		pEnt = NULL;

		return true;
	}

	Entity* EntityManager::GetEntityFromPoint(const glm::vec3& pos, const glm::vec3& origin)
	{
		for (Lamp::Entity* ent : GetEntities())
		{
			Ray ray;
			ray.origin = origin;
			ray.direction = pos;

			if (ent->GetPhysicalEntity()->GetCollider()->IntersectRay(ray).IsIntersecting)
			{
				return ent;
			}
		}

		return nullptr;
	}
}