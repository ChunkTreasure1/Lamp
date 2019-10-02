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
		IEntity* CreateEntity(const glm::vec2& pos, const std::string& path)
		{
			IEntity* pEnt = new IEntity();
			pEnt->SetPosition((uint32_t)m_pEntites.size()); //CHANGE
			pEnt->GetOrCreateComponent<TransformComponent>(pos);
			pEnt->GetOrCreateComponent<SpriteComponent>(path);

			m_pEntites.emplace_back(pEnt);

			return pEnt;
		}
		bool RemoveEntity(IEntity* pEnt)
		{
			pEnt->Destroy();
			m_pEntites.erase(m_pEntites.begin() + pEnt->GetPosition());
			delete pEnt;

			return true;
		}

	private:
		std::vector<IEntity*> m_pEntites;
	};
}