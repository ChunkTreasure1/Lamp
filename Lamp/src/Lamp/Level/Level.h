#pragma once

#include <string>

#include "Lamp/Objects/Brushes/Brush2D.h"
#include "Lamp/Objects/Brushes/BrushManager.h"
#include "Lamp/Objects/Entity/Base/EntityManager.h"
#include "Lamp/Objects/ObjectLayer.h"
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

	class Level
	{
	public:
		Level(const std::string& name, const std::string& path)
			: m_Name(name), m_Path(path)
		{
		}

		~Level()
		{
		}

		inline LevelEnvironment& GetEnvironment() { return m_Environment; }
		inline const std::string& GetName() { return m_Name; }
		inline const std::string& GetPath() { return m_Path; }
		inline void SetPath(const std::string& path) { m_Path = path; }

	private:
		std::string m_Name;
		std::string m_Path;
		LevelEnvironment m_Environment;
	};
}