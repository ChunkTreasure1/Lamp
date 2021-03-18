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
		inline const std::unordered_map<uint32_t, Entity*>& GetEntities() { return m_pEntities; }

		Entity* GetEntityFromId(uint32_t id);
		Entity* GetEntityFromPoint(const glm::vec3& pos, const glm::vec3& origin);

		//Setting
		inline void SetEntities(std::unordered_map<uint32_t, Entity*> entities) { m_pEntities = entities; }

	private:
		std::unordered_map<uint32_t, Entity*> m_pEntities;
	};
}