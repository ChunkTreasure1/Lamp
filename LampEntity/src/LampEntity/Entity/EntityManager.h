#pragma once

#include <vector>
#include <memory>
#include <Lamp/Rendering/Texture/GLTexture.h>

#include "Entity.h"

namespace LampEntity
{
	class EntityManager
	{
	public:
		void Update();
		void Draw();
		void Refresh();

		std::unique_ptr<IEntity>& CreateEntity(glm::vec2& pos, const std::string& path);

		//Getting
		inline const std::vector<std::unique_ptr<IEntity>>& GetEntities() const { return m_pEntities; }

	private:
		std::vector<std::unique_ptr<IEntity>> m_pEntities;  
	};
}