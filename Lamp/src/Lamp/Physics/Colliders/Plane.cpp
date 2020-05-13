#include "lppch.h"
#include "Plane.h"

#include "BoundingSphere.h"

namespace Lamp
{
	Plane Plane::Normalized() const
	{
		float mag = m_Normal.length();

		return Plane(m_Normal / mag, m_Distance / mag);
	}

	IntersectData Plane::IntersectBoundingSphere(const Ref<BoundingSphere>& other)
	{
		float distFromCenter = glm::abs(glm::dot(m_Normal, other->GetCenter()) - m_Distance);
		float distFromSphere = distFromCenter - other->GetRadius();
	
		return IntersectData(distFromSphere < 0, distFromSphere);
	}
}