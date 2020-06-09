#pragma once

#include <vector>

#include "Lamp/Entity/Base/Entity.h"

namespace Lamp
{
	class EntityManager
	{
	public:
		EntityManager() {}
		void OnEvent(Event& e);

		void Shutdown();
		Entity* Create();
		bool Remove(Entity* pEnt);

		//Getting
		inline const std::vector<Entity*> GetEntities() { return m_pEntites; }
		Entity* GetEntityFromPhysicalEntity(PhysicalEntity* pEnt);

		//Setting
		inline void SetEntities(std::vector<Entity*> entities) { m_pEntites = entities; }

	public:
		Entity* GetEntityFromPoint(const glm::vec3& pos, const glm::vec3& origin);

	public:
		static void SetCurrentManager(Ref<EntityManager> manager) { s_CurrentManager = manager; }
		static Ref<EntityManager>& Get() { return s_CurrentManager; }

	private:
		std::vector<Entity*> m_pEntites;

	private:
		static Ref<EntityManager> s_CurrentManager;
	};
}