#include "lppch.h"
#include "BoundingSphere.h"

namespace Lamp
{
	IntersectData BoundingSphere::IntersectRay(const Ray& ray) const
	{
		return IntersectData(false, glm::vec3(0.f));
	}

	IntersectData BoundingSphere::Intersect(const Ref<Collider>& other) const
	{
		if (other->GetType() == CollType::Sphere)
		{
			Ref<BoundingSphere> coll = std::dynamic_pointer_cast<BoundingSphere>(other);

			float radDist = m_Radius + coll->GetRadius();
			float centerDist = glm::length(coll->GetCenter() - m_Center);
			
			glm::vec3 direction = coll->GetCenter() - m_Center;
			direction /= centerDist;

			float distance = centerDist - radDist;

			return IntersectData(distance < 0, direction * distance);
		}

		return IntersectData(false, glm::vec3(0.f));
	}

	void BoundingSphere::Transform(const glm::vec3& translation)
	{
		m_Center += translation;
	}
}