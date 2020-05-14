#include "lppch.h"
#include "AABB.h"

#include "Lamp/Core/Math.h"

namespace Lamp
{
	IntersectData AABB::IntersectRay(const Ray& ray) const
	{
		return IntersectData(false, glm::vec3(0.f));
	}

	IntersectData AABB::Intersect(const Ref<Collider>& other) const
	{
		if (other->GetType() == CollType::AABB)
		{
			Ref<AABB> coll = std::dynamic_pointer_cast<AABB>(other);

			glm::vec3 distancesA = coll->GetMinExtents() - GetMaxExtents();
			glm::vec3 distancesB = GetMinExtents() - coll->GetMaxExtents();

			glm::vec3 distances = glm::max(distancesA, distancesB);

			float maxDistance = Math::Max(distances);

			return IntersectData(maxDistance < 0, distances);
		}

		return IntersectData(false, glm::vec3(0.f));
	}

	void AABB::Transform(const glm::vec3& translation)
	{
		m_MaxExtents += translation;
		m_MinExtents += translation;
	}
}