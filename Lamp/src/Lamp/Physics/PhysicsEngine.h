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
		void Simulate(float delta);

	private:
		std::vector<Ref<PhysicalEntity>> m_PhysicalEntites;
	};
}