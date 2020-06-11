#pragma once

#include <string>

#include "Lamp/Objects/Brushes/Brush2D.h"
#include "Lamp/Objects/Brushes/BrushManager.h"

#include "Lamp/Objects/Entity/Base/EntityManager.h"

namespace Lamp
{
	class Level
	{
	public:
		Level(const std::string& name)
			: m_Name(name)
		{
			m_BrushManager = std::make_shared<BrushManager>();
			m_EntityManager = std::make_shared<EntityManager>();
		}

		inline Ref<BrushManager>& GetBrushManager() { return m_BrushManager; }
		inline Ref<EntityManager>& GetEntityManager() { return m_EntityManager; }
		inline const std::string& GetName() { return m_Name; }

	private:
		std::string m_Name;
		Ref<BrushManager> m_BrushManager;
		Ref<EntityManager> m_EntityManager;
	};
}