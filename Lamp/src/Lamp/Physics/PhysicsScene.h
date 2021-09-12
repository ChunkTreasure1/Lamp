#pragma once

#include "PhysicsSettings.h"

namespace Lamp
{
	struct RaycastHit
	{
		uint64_t HitEntity;
		glm::vec3 Position;
		glm::vec3 Normal;
		float Distance;
	};

	class PhysicsScene
	{
	public:
		PhysicsScene(const PhysicsSettings& settings);
		~PhysicsScene();

		void Simulate(float ts, bool callFixedUpdate = true);

		Ref<PhysicsActor> GetActor(Entity* pEnt);
	};
}