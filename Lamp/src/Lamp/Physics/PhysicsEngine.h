#pragma once

#include <vector>
#include "Lamp/Entity/Base/Physical/PhysicalEntity.h"
#include "Lamp/Core/Core.h"

namespace Lamp
{
	class PhysicsEngine
	{
	public:
		PhysicsEngine() {}

		void AddEntity(Ref<PhysicalEntity>& entity);
		void RemoveEntity(Ref<PhysicalEntity>& entity);

		void Simulate(float delta);
		void HandleCollisions();

		static Ref<PhysicsEngine>& Get() { return s_PhysicsEngine; }

	private:
		std::vector<Ref<PhysicalEntity>> m_PhysicalEntites;

	private:
		static Ref<PhysicsEngine> s_PhysicsEngine;
	};
}