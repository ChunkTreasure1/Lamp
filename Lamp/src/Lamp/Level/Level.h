#pragma once

#include <string>

#include "Lamp/Objects/Brushes/Brush2D.h"
#include "Lamp/Objects/Brushes/BrushManager.h"
#include "Lamp/Objects/Entity/Base/EntityManager.h"
#include "Lamp/Objects/ObjectLayer.h"
#include "Lamp/Physics/PhysicsEngine.h"

namespace Lamp
{
	class Level
	{
	public:
		Level(const std::string& name)
			: m_Name(name)
		{
			m_ObjectLayerManager = std::make_shared<ObjectLayerManager>();
			m_BrushManager = std::make_shared<BrushManager>();
			m_EntityManager = std::make_shared<EntityManager>();
			m_PhysicsEngine = std::make_shared<PhysicsEngine>();
		}

		~Level()
		{
			m_ObjectLayerManager->Destroy();
		}

		inline Ref<BrushManager>& GetBrushManager() { return m_BrushManager; }
		inline Ref<EntityManager>& GetEntityManager() { return m_EntityManager; }
		inline Ref<ObjectLayerManager>& GetObjectLayerManager() { return m_ObjectLayerManager; }
		inline Ref<PhysicsEngine>& GetPhysicsEngine() { return m_PhysicsEngine; }

		inline const std::string& GetName() { return m_Name; }

	private:
		std::string m_Name;
		Ref<BrushManager> m_BrushManager;
		Ref<EntityManager> m_EntityManager;
		Ref<ObjectLayerManager> m_ObjectLayerManager;
		Ref<PhysicsEngine> m_PhysicsEngine;
	};
}