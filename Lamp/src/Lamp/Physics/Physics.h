#pragma once

#include "Collision.h"
#include "SpherePhysicsObject.h"

namespace Lamp
{
	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;
	};

	class Physics
	{
	public:

		static void CheckCollisions() { Collision::CheckCollisions(); }
		static bool IntersectRaySphere(const Ray& ray, SpherePhysicsObject& obj);

		//Getting
		static const float GetGravity() { return m_Gravity; }

		//Setting
		static inline void SetGravity(float val) { m_Gravity = val; }

	private:
		Physics() = delete;

		static float m_Gravity;
	};

}