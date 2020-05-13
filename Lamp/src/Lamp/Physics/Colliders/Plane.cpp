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

	IntersectData Plane::IntersectRay(const Ray& ray) const
	{
		return IntersectData(false, glm::vec3(0.f));
	}

	IntersectData Plane::Intersect(const Ref<Collider>& other) const
	{
		if (other->GetType() == CollType::Sphere)
		{
			Ref<BoundingSphere> coll = std::dynamic_pointer_cast<BoundingSphere>(other);

			float distFromCenter = glm::abs(glm::dot(m_Normal, coll->GetCenter()) - m_Distance);
			float distFromSphere = distFromCenter - coll->GetRadius();

			return IntersectData(distFromSphere < 0, m_Normal * distFromSphere);
		}

		return IntersectData(false, glm::vec3(0.f));
	}

	void Plane::Transform(const glm::vec3& translation)
	{
		m_Normal += translation;
	}
}