#pragma once

#include <glm/glm.hpp>
#include "Collider.h"

namespace Lamp
{
	class Plane : public Collider
	{
	public:
		Plane(const glm::vec3& normal, float dist)
			: Collider(CollType::Plane), m_Normal(normal), m_Distance(dist)
		{}

		Plane Normalized() const;
		virtual IntersectData IntersectRay(const Ray& ray) const override;
		virtual IntersectData Intersect(const Ref<Collider>& other) const override;
		virtual void Transform(const glm::vec3& translation) override;

		//Getting
		inline const glm::vec3& GetNormal() const { return m_Normal; }
		inline const float GetDistance() const { return m_Distance; }

	private:
		glm::vec3 m_Normal;
		float m_Distance;
	};
}