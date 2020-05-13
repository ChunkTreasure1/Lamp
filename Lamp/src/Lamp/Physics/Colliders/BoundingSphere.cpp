#include "lppch.h"
#include "BoundingSphere.h"

namespace Lamp
{
	IntersectData BoundingSphere::IntersectRay(const Ray& ray) const
	{
		return IntersectData(false, 0.f);
	}

	IntersectData BoundingSphere::IntersectBoundingSphere(const Ref<BoundingSphere>& other) const
	{

		float radDist = m_Radius + other->GetRadius();
		float centerDist = glm::length(other->GetCenter() - m_Center);

		if (centerDist < radDist)
		{
			return IntersectData(true, centerDist - radDist);
		}

		return IntersectData(false, centerDist - radDist);
	}
}