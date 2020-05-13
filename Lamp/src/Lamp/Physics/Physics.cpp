#include "lppch.h"
#include "Physics.h"

#include "Lamp/Core/Math.h"

namespace Lamp
{
	float Physics::m_Gravity = 9.81f;

	std::pair<float, float> Physics::GetVelocities(Ref<PhysicalEntity>& coll1, Ref<PhysicalEntity>& coll2)
	{
		float mass1, mass2;
		float v1, v2;
		float v1a, v2a;

		mass1 = coll1->GetMass();
		mass2 = coll2->GetMass();

		v1 = glm::length(coll1->GetVelocity());
		v2 = glm::length(coll2->GetVelocity());

		float pBefore = mass1 * v1 + mass2 * v2;
		float wBefore = ((mass1 * v1 * v1) / 2) + ((mass2 * v2 * v2) / 2);

		

		return std::pair<float, float>();
	}
}