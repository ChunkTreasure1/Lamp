#include "lppch.h"
#include "AABB.h"

#include "Lamp/Core/Math.h"

namespace Lamp
{
	IntersectData AABB::IntersectRay(const Ray& ray) const
	{
		float tMin = (m_MinExtents.x - ray.origin.x) / ray.direction.x;
		float tMax = (m_MaxExtents.x - ray.origin.x) / ray.direction.x;

		if (tMin > tMax)
		{
			std::swap(tMin, tMax);
		}

		float tyMin = (m_MinExtents.y - ray.origin.y) / ray.direction.y;
		float tyMax = (m_MaxExtents.y - ray.origin.y) / ray.direction.y;

		if (tyMin > tyMax)
		{
			std::swap(tyMin, tyMax);
		}

		if ((tMin > tyMax) || (tyMin > tMax))
		{
			return IntersectData(false, glm::vec3(0.f));
		}

		if (tyMin > tMin)
		{
			tMin = tyMin;
		}

		if (tyMax < tMax)
		{
			tMax = tyMax;
		}

		float tzMin = (m_MinExtents.z - ray.origin.z) / ray.direction.z;
		float tzMax = (m_MaxExtents.z - ray.origin.z) / ray.direction.z;

		if (tzMin > tzMax)
		{
			std::swap(tzMin, tzMin);
		}

		if ((tMin > tzMax) || tzMin > tMax)
		{
			return IntersectData(false, glm::vec3(0.f));
		}

		if (tzMin > tMin)
		{
			tMin = tzMin;
		}

		if (tzMax < tMax)
		{
			tMax = tzMax;
		}

		return IntersectData(true, glm::vec3(0.f));
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