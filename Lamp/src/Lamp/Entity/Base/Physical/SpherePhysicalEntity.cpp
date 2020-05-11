#include "lppch.h"
#include "SpherePhysicalEntity.h"

#include "Lamp/Core/Math.h"
#include "Lamp/Entity/Base/Entity.h"
#include "Lamp/Brushes/Brush.h"

namespace Lamp
{
	bool SpherePhysicalEntity::Intersect(const Ray& ray) const
	{
		float t0 = 0.f;
		float t1 = 0.f;

		glm::vec3 pos;
		if (m_pEntity)
		{
			pos = m_pEntity->GetPosition();
		}
		else if (m_pBrush)
		{
			pos = m_pBrush->GetPosition();
		}

		glm::vec3 l = ray.origin - pos;
		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.f * glm::dot(ray.direction, l);
		float c = glm::dot(l, l) - m_RadiusSqr;

		if (!Math::SolveQuadric(a, b, c, t0, t1))
		{
			return false;
		}

		if (t0 > t1)
		{
			std::swap(t0, t1);
		}

		if (t0 < 0)
		{
			t0 = t1;
			if (t0 < 0)
			{
				return false;
			}
		}


		LP_INFO("Raycast Hit!");
		return true;
	}
}