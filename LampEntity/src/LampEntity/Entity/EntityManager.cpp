#include "EntityManager.h"

#include "Entity.h"

#include "LampEntity/BaseComponents/SpriteComponent.h"
#include "LampEntity/BaseComponents/TransformComponent.h"

namespace LampEntity
{
	void EntityManager::Update()
	{
		for (auto& pEnt : m_pEntities)
		{
			pEnt->Update();
		}
	}

	void EntityManager::Draw()
	{
		for (auto& pEnt : m_pEntities)
		{
			pEnt->Draw();
		}
	}

	void EntityManager::Refresh()
	{
		m_pEntities.erase(std::remove_if(std::begin(m_pEntities), std::end(m_pEntities), [](const std::unique_ptr<IEntity> &pEntity)
		{
			return !pEntity->GetIsActive();
		}),
		std::end(m_pEntities));
	}

	std::unique_ptr<IEntity>& EntityManager::CreateEntity(glm::vec2& pos, const std::string& path)
	{
		IEntity* pEnt = new IEntity();
		std::unique_ptr<IEntity> pPtr{ pEnt };

		auto pTrans = pEnt->GetOrCreateComponent<TransformComponent>();
		pTrans->SetPosition(pos);

		auto pSprite = pEnt->GetOrCreateComponent<SpriteComponent>(path);

		m_pEntities.emplace_back(std::move(pEnt));

		return pPtr;
	}
}