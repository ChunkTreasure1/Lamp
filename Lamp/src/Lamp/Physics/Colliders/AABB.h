#pragma once

#include "Collider.h"

namespace Lamp
{
	class AABB : public Collider
	{
	public:
		AABB(const glm::vec3& min, const glm::vec3& max)
			: Collider(CollType::AABB), m_MinExtents(min), m_MaxExtents(max)
		{}

		virtual IntersectData IntersectRay(const Ray& ray) const override;
		virtual IntersectData Intersect(const Ref<Collider>& other) const override;
		virtual void Transform(const glm::vec3& translation) override;

		inline const glm::vec3& GetMinExtents() const { return m_MinExtents; }
		inline const glm::vec3& GetMaxExtents() const { return m_MaxExtents; }

	private:
		glm::vec3 m_MinExtents;
		glm::vec3 m_MaxExtents;
	};
}