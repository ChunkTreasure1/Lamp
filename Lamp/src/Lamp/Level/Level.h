#pragma once

#include <string>

#include "Lamp/Brushes/Brush2D.h"
#include "Lamp/Brushes/BrushManager.h"

#include "Lamp/Entity/Base/EntityManager.h"

namespace Lamp
{
	class Level
	{
	public:
		Level(const std::string& name)
			: m_Name(name)
		{}

		inline BrushManager& GetBrushManager() { return m_BrushManager; }
		inline EntityManager& GetEntityManager() { return m_EntityManager; }
		inline const std::string& GetName() { return m_Name; }

	private:
		std::string m_Name;
		BrushManager m_BrushManager;
		EntityManager m_EntityManager;
	};
}