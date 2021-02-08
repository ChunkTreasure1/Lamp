#pragma once

#include <vector>

#include "Lamp/Objects/Entity/Base/Entity.h"

namespace Lamp
{
	class EntityManager
	{
	public:
		EntityManager();
		~EntityManager();

		Entity* Create(bool saveable = true);
		bool Remove(Entity* pEnt);

		//Getting
		inline const std::vector<Entity*>& GetEntities() { return m_pEntites; }
<<<<<<< HEAD
=======
		Entity* GetEntityFromPhysicalEntity(PhysicalEntity* pEnt);
		Entity* GetEntityFromId(uint32_t id);
		Entity* GetEntityFromPoint(const glm::vec3& pos, const glm::vec3& origin);
>>>>>>> renderer

		//Setting
		inline void SetEntities(std::vector<Entity*> entities) { m_pEntites = entities; }

	public:
		static EntityManager* Get() { return s_CurrentManager; }

	private:
		std::vector<Entity*> m_pEntites;

	private:
		static EntityManager* s_CurrentManager;
	};
}