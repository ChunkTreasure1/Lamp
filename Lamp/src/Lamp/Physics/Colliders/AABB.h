#pragma once

#include "Collider.h"

namespace Lamp
{
	class AABB : public Collider
	{
	public:
		AABB(const glm::vec3& min, const glm::vec3& max)
			: m_MinExtents(min), m_MaxExtents(max)
		{}

		virtual IntersectData IntersectAABB(const Ref<AABB>& other) const override;

		inline const glm::vec3& GetMinExtents() const { return m_MinExtents; }
		inline const glm::vec3& GetMaxExtents() const { return m_MaxExtents; }

	private:
		glm::vec3 m_MinExtents;
		glm::vec3 m_MaxExtents;
	};
}