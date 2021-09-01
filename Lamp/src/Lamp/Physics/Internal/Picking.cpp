#include "lppch.h"
#include "Picking.h"

namespace Lamp
{
	const bool PickingCollider::Intersect(const Ray& r) const
	{
		glm::vec3 invDir = { 1 / r.dir.x, 1 / r.dir.y, 1 / r.dir.z };

		float tMin, tMax, tyMin, tyMax, tzMin, tzMax;

		if (invDir.x >= 0)
		{
			tMin = (m_WorldMin.x - r.origin.x) * invDir.x;
			tMax = (m_WorldMax.x - r.origin.x) * invDir.x;
		}
		else
		{
			tMin = (m_WorldMax.x - r.origin.x) * invDir.x;
			tMax = (m_WorldMin.x - r.origin.x) * invDir.x;
		}

		if (invDir.y >= 0)
		{
			tyMin = (m_WorldMin.y - r.origin.y) * invDir.y;
			tyMax = (m_WorldMax.y - r.origin.y) * invDir.y;
		}
		else
		{
			tyMin = (m_WorldMax.y - r.origin.y) * invDir.y;
			tyMax = (m_WorldMin.y - r.origin.y) * invDir.y;
		}

		if ((tMin > tyMax) || (tyMin > tMax))
		{
			return false;
		}

		if (tyMin > tMin)
		{
			tMin = tyMin;
		}

		if (tyMax < tMax)
		{
			tMax = tyMax;
		}

		if (invDir.z >= 0)
		{
			tzMin = (m_WorldMin.z - r.origin.z) * invDir.z;
			tzMax = (m_WorldMax.z - r.origin.z) * invDir.z;
		}
		else
		{
			tzMin = (m_WorldMax.z - r.origin.z) * invDir.z;
			tzMax = (m_WorldMin.z - r.origin.z) * invDir.z;
		}

		if ((tMin > tzMax) || tzMin > tMax)
		{
			return false;
		}

		if (tzMin > tMin)
		{
			tMin = tzMin;
		}

		if (tzMax < tMax)
		{
			tMax = tzMax;
		}

		return true;
	}

	void PickingCollider::Transform(const glm::vec3& trans)
	{
		m_WorldMax += trans;
		m_WorldMin += trans;
	}
}