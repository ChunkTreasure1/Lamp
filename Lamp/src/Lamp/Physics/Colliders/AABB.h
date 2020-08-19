#pragma once

#include "Collider.h"

namespace Lamp
{
	class AABB : public Collider
	{
	public:
		AABB(const glm::vec3& min, const glm::vec3& max)
			: Collider(CollType::AABB), m_MinExtents(min), m_MaxExtents(max), m_WorldMin(min), m_WorldMax(max)
		{}

		virtual IntersectData IntersectRay(const Ray& ray) const override;
		virtual IntersectData Intersect(const Ref<Collider>& other) const override;
		virtual void Transform(const glm::vec3& translation) override;
		virtual void SetTranslation(const glm::vec3& trans) override;
		virtual void Render() override;

		inline const glm::vec3& GetMinExtents() const { return m_MinExtents; }
		inline const glm::vec3& GetMaxExtents() const { return m_MaxExtents; }
		inline const glm::vec3& GetWorldMin() const { return m_WorldMin; }
		inline const glm::vec3& GetWorldMax() const { return m_WorldMax; }

	private:
		glm::vec3 m_MinExtents;
		glm::vec3 m_MaxExtents;

		glm::vec3 m_WorldMax;
		glm::vec3 m_WorldMin;
	};
}