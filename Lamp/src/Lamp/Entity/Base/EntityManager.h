#pragma once

#include <vector>

#include "Lamp/Entity/Base/Entity.h"
#include "Lamp/Entity/BaseComponents/TransformComponent.h"
#include "Lamp/Entity/BaseComponents/SpriteComponent.h"

namespace Lamp
{
	class EntityManager
	{
	public:
		EntityManager() {}

		void Update()
		{
			for (IEntity* pE : m_pEntites)
			{
				pE->Update();
			}
		}
		void Draw()
		{
			for (IEntity* pE : m_pEntites)
			{
				pE->Draw();
			}
		}

		void Shutdown()
		{
			for (size_t i = 0; i < m_pEntites.size(); i++)
			{
				delete m_pEntites[i];
			}

			m_pEntites.clear();
		}
		IEntity* CreateEntity(const glm::vec3& pos, const std::string& path)
		{
			IEntity* pEnt = new IEntity();
			pEnt->GetOrCreateComponent<TransformComponent>(pos);
			pEnt->GetOrCreateComponent<SpriteComponent>(path);

			m_pEntites.emplace_back(pEnt);

			return pEnt;
		}
		bool RemoveEntity(IEntity* pEnt)
		{
			pEnt->Destroy();
			auto it = std::find(m_pEntites.begin(), m_pEntites.end(), pEnt);
			if (it != m_pEntites.end())
			{
				m_pEntites.erase(it);
			}
			delete pEnt;

			return true;
		}

		//Getting
		inline const std::vector<IEntity*> GetEntities() const { return m_pEntites; }

	public:
		static IEntity* GetEntityFromPoint(const glm::vec2& pos, const std::shared_ptr<EntityManager>& pEntityManager) 
		{
			for (Lamp::IEntity* pEnt : pEntityManager->GetEntities())
			{
				if (auto pTrans = pEnt->GetComponent<Lamp::TransformComponent>())
				{
					glm::vec4 rect(pTrans->GetPosition().x, pTrans->GetPosition().y, pTrans->GetScale().x, pTrans->GetScale().y);

					if ((pos.x > (rect.x - (rect.z / 2))) &&
						 pos.x < (rect.x + (rect.z / 2)) &&
						 
						 pos.y > (rect.y - (rect.w / 2)) &&
						 pos.y < (rect.y + (rect.w / 2)))
					{
						return pEnt;
					}
				}
			}

			return nullptr;
		}

	private:
		std::vector<IEntity*> m_pEntites;
	};
}