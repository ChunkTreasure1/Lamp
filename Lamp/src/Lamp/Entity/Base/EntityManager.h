#pragma once

#include <vector>

#include "Lamp/Entity/Base/Entity.h"

namespace Lamp
{
	class EntityManager
	{
	public:
		EntityManager() {}

		void Update(Timestep ts);
		void Draw();
		void OnEvent(Event& e);

		void Shutdown();
		IEntity* Create();
		bool Remove(IEntity* pEnt);

		//Getting
		inline const std::vector<IEntity*> GetEntities() { return m_pEntites; }

		//Setting
		inline void SetEntities(std::vector<IEntity*> entities) { m_pEntites = entities; }

	public:
		IEntity* GetEntityFromPoint(const glm::vec2& pos);

	public:
		static void SetCurrentManager(EntityManager& manager) { s_CurrentManager = manager; }
		static EntityManager& Get() { return s_CurrentManager; }

	private:
		std::vector<IEntity*> m_pEntites;

	private:
		static EntityManager s_CurrentManager;
	};
}