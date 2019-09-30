#include "EntityManager.h"

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

	IEntity& EntityManager::CreateEntity(glm::vec2& pos, const std::string& path)
	{
		std::unique_ptr<IEntity> pEnt = std::make_unique<IEntity>();

		auto* pTrans = pEnt->GetOrCreateComponent<TransformComponent>();
		pTrans->SetPosition(pos);

		auto* pSprite = pEnt->GetOrCreateComponent<SpriteComponent>(path);

		m_pEntities.emplace_back(std::move(pEnt));

		return *pEnt.get();
	}
}