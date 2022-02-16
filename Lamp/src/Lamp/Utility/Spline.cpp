#include "lppch.h"
#include "Spline.h"

namespace Lamp
{
	Spline::Spline()
	{
	}

	Spline::Spline(const std::vector<glm::vec3>& positions)
		: m_points(positions)
	{
	}

	void Spline::AddPoint(const glm::vec3& point)
	{
		m_points.emplace_back(point);
	}
}