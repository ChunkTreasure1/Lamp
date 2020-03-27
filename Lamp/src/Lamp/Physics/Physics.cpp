#include "lppch.h"
#include "Physics.h"

#include "Lamp/Core/Math.h"

namespace Lamp
{
	float Physics::m_Gravity = 9.81f;

	bool Physics::IntersectRaySphere(const Ray& ray, SpherePhysicsObject& obj)
	{
		float t0 = 0.f;
		float t1 = 0.f;

		glm::vec3 l = ray.origin - obj.GetPosition();
		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.f * glm::dot(ray.direction, l);
		float c = glm::dot(l, l) - obj.GetRadiusSqr();

		if (!Math::SolveQuadric(a, b, c, t0, t1))
		{
			return false;
		}

		if (t0 > t1)
		{
			std::swap(t0, t1);
		}

		if (t0 < 0)
		{
			t0 = t1;
			if (t0 <0)
			{
				return false;
			}
		}



		return true;
	}
}