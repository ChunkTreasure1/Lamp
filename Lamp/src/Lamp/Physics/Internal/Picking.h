#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 dir;
	};

	class PickingCollider
	{
	public:
		PickingCollider(const glm::vec3& min, const glm::vec3& max, const glm::vec3& pos) 
			: m_MaxExtents(max), m_MinExtents(min), m_WorldMax(max + pos), m_WorldMin(min + pos)
		{
			m_MaxExtents.y += 0.5f;
			m_MinExtents.y += 0.5f;
		}

		const bool Intersect(const Ray& r) const;
		void Transform(const glm::vec3& trans);

	private:
		glm::vec3 m_MinExtents;
		glm::vec3 m_MaxExtents;
	
		glm::vec3 m_WorldMax;
		glm::vec3 m_WorldMin;
	};
}