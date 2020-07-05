#include "lppch.h"
#include "AABB.h"

#include "Lamp/Core/Math.h"
#include "Lamp/Rendering/Renderer2D.h"

namespace Lamp
{
	IntersectData AABB::IntersectRay(const Ray& ray) const
	{
		glm::vec3 invDir = { 1 / ray.direction.x, 1 / ray.direction.y, 1/ ray.direction.z };

		float tMin, tMax, tyMin, tyMax, tzMin, tzMax;

		if (invDir.x >= 0)
		{
			tMin = (m_MinExtents.x - ray.origin.x) * invDir.x;
			tMax = (m_MaxExtents.x - ray.origin.x) * invDir.x;
		}
		else 
		{
			tMin = (m_MaxExtents.x - ray.origin.x) * invDir.x;
			tMax = (m_MinExtents.x - ray.origin.x) * invDir.x;
		}

		if (invDir.y >= 0)
		{
			tyMin = (m_MinExtents.y - ray.origin.y) * invDir.y;
			tyMax = (m_MaxExtents.y - ray.origin.y) * invDir.y;
		}
		else
		{
			tyMin = (m_MaxExtents.y - ray.origin.y) * invDir.y;
			tyMax = (m_MinExtents.y - ray.origin.y) * invDir.y;
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

		if (invDir.z >= 0)
		{
			tzMin = (m_MinExtents.z - ray.origin.z) * invDir.z;
			tzMax = (m_MaxExtents.z - ray.origin.z) * invDir.z;
		}
		else 
		{
			tzMin = (m_MaxExtents.z - ray.origin.z) * invDir.z;
			tzMax = (m_MinExtents.z - ray.origin.z) * invDir.z;
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

			glm::vec3 distancesA = coll->GetWorldMin() - GetWorldMax();
			glm::vec3 distancesB = GetWorldMin() - coll->GetWorldMax();

			glm::vec3 distances = glm::max(distancesA, distancesB);

			float maxDistance = Math::Max(distances);

			return IntersectData(maxDistance < 0, distances);
		}

		return IntersectData(false, glm::vec3(0.f));
	}

	void AABB::Transform(const glm::vec3& translation)
	{
		m_WorldMax += translation;
		m_WorldMin += translation;
	}

	void AABB::SetTranslation(const glm::vec3& trans)
	{
		m_WorldMax = trans + m_MaxExtents;
		m_WorldMin = trans + m_MinExtents;
	}

	void AABB::Render()
	{
		Lamp::Renderer2D::DrawLine({ m_WorldMax.x, m_WorldMax.y }, { m_WorldMin.x, m_WorldMin.y });
	}
}