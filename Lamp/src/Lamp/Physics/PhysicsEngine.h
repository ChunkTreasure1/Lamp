#pragma once

#include <vector>
#include "Lamp/Objects/Entity/Base/Physical/PhysicalEntity.h"
#include "Lamp/Core/Core.h"

namespace Lamp
{
	class PhysicsEngine
	{
	public:
		PhysicsEngine() {}
		~PhysicsEngine()
		{
			float a = 0.f;
		}

		void AddEntity(Ref<PhysicalEntity>& entity);
		void RemoveEntity(Ref<PhysicalEntity>& entity);

		void Simulate(float delta);
		void HandleCollisions();

	public:
		static void SetCurrentEngine(Ref<PhysicsEngine> engine) { s_PhysicsEngine = engine; }
		static Ref<PhysicsEngine>& Get() { return s_PhysicsEngine; }

	private:
		std::vector<Ref<PhysicalEntity>> m_PhysicalEntites;

	private:
		static Ref<PhysicsEngine> s_PhysicsEngine;
	};
}