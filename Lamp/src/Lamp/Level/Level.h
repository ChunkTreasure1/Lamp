#pragma once

#include <string>
#include "Lamp/Objects/Brushes/Brush.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include <btBulletDynamicsCommon.h>

namespace Lamp
{
	struct LevelEnvironment
	{
		glm::vec3 GlobalAmbient{ 0.3f, 0.3f, 0.3f };
		glm::vec3 CameraPosition{ 0.f, 0.f, 0.f };
		glm::vec3 CameraRotation{ 0.f, 0.f, 0.f };

		float CameraFOV = 60.f;
	};

	struct LevelLoadData
	{
		std::string name;
		std::string path;

		LevelEnvironment env;
	};

	class Level
	{
	public:
		Level(const std::string& name, const std::string& path)
			: m_Name(name), m_Path(path)
		{
		}

		Level(const Level& level)
		{
			for (auto& brush : level.m_Brushes)
			{
				m_Brushes.emplace(std::make_pair(brush.first, Brush::Duplicate(brush.second, false)));
			}

			for (auto& entity : level.m_Entities)
			{
				m_Entities.emplace(std::make_pair(entity.first, Entity::Duplicate(entity.second, false)));
			}

			m_Environment = level.m_Environment;
			m_Name = m_Name;
			m_Path = m_Path;
		}

		~Level()
		{
		}

		inline LevelEnvironment& GetEnvironment() { return m_Environment; }
		inline const std::string& GetName() { return m_Name; }
		inline const std::string& GetPath() { return m_Path; }
		inline void SetPath(const std::string& path) { m_Path = path; }
		inline std::map<uint32_t, Brush*>& GetBrushes() { return m_Brushes;  }
		inline std::map<uint32_t, Entity*>& GetEntities() { return m_Entities;  }

		void OnEvent(Event& e);
		void UpdateEditor(Timestep ts, Ref<CameraBase>& camera);
		void UpdateRuntime(Timestep ts);

	private:
		std::string m_Name;
		std::string m_Path;
		LevelEnvironment m_Environment;
		std::map<uint32_t, Brush*> m_Brushes;
		std::map<uint32_t, Entity*> m_Entities;
	};
}