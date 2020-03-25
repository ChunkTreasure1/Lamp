#include "lppch.h"
#include "EntityManager.h"

namespace Lamp
{
	Ref<EntityManager> EntityManager::s_CurrentManager;

	void EntityManager::Update(Timestep ts)
	{
		for (int i = 0; i < m_pEntites.size(); i++)
		{
			m_pEntites[i]->Update(ts);
		}
	}

	void EntityManager::Draw()
	{
		for (IEntity* pE : m_pEntites)
		{
			pE->Draw();
		}
	}

	void EntityManager::OnEvent(Event& e)
	{
		for (IEntity* pE : m_pEntites)
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

	IEntity* EntityManager::Create()
	{
		IEntity* pEnt = new IEntity();
		m_pEntites.emplace_back(pEnt);

		return pEnt;
	}

	bool EntityManager::Remove(IEntity* pEnt)
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
		delete pEnt;

		pEnt = NULL;

		return true;
	}

	IEntity* EntityManager::GetEntityFromPoint(const glm::vec2& pos)
	{
		for (Lamp::IEntity* pEnt : GetEntities())
		{
			glm::vec4 rect(pEnt->GetPosition().x, pEnt->GetPosition().y, pEnt->GetScale().x, pEnt->GetScale().y);

			if ((pos.x > (rect.x - (rect.z / 2))) &&
				pos.x < (rect.x + (rect.z / 2)) &&

				pos.y >(rect.y - (rect.w / 2)) &&
				pos.y < (rect.y + (rect.w / 2)))
			{
				return pEnt;
			}
		}

		return nullptr;
	}
}