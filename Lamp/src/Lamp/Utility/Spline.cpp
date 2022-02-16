#include "lppch.h"
#include "Spline.h"

namespace Lamp
{
	void Spline::AddPoint(const glm::vec3& point)
	{
		m_points.emplace_back(point);
	}
}