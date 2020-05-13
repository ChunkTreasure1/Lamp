#include "lppch.h"
#include "AABB.h"

#include "Lamp/Core/Math.h"

namespace Lamp
{
	IntersectData AABB::IntersectAABB(const Ref<AABB>& other) const
	{
		glm::vec3 distancesA = other->GetMinExtents() - GetMaxExtents();
		glm::vec3 distancesB = GetMinExtents() - other->GetMaxExtents();

		glm::vec3 distances = glm::max(distancesA, distancesB);

		float maxDistance = Math::Max(distances);

		return IntersectData(maxDistance < 0, maxDistance);
	}
}