#include "lppch.h"
#include "Physics.h"

#include "Lamp/Core/Math.h"

namespace Lamp
{
	float Physics::m_Gravity = 9.81f;

	float Physics::GetVelocities(Ref<PhysicalEntity>& coll1, Ref<PhysicalEntity>& coll2)
	{
		float mass1, mass2 = 0.f;
		float v1, v2 = 0.f;
		float v1a = 0.f;

		mass1 = coll1->GetMass();
		mass2 = coll2->GetMass();

		v1 = glm::length(coll1->GetVelocity());
		v2 = glm::length(coll2->GetVelocity());

		if (coll1->GetVelocity().x < 0 || coll1->GetVelocity().y < 0 || coll1->GetVelocity().z < 0)
		{
			v1 *= -1;
		}

		if (coll2->GetVelocity().x < 0 || coll2->GetVelocity().y < 0 || coll2->GetVelocity().z < 0)
		{
			v2 *= -1;
		}

		if (mass1 + mass2 > 0)
		{
			v1a = (mass1 * v1 + mass2 * v2) / (mass1 + mass2);

			LP_CORE_INFO("Vel1: {0}", v1a);
		}

		return v1a;
	}
}