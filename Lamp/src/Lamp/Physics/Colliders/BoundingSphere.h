#pragma once

#include <glm/glm.hpp>
#include "Collider.h"
#include "Lamp/Core/Core.h"

#include "lppch.h"

namespace Lamp
{
	class BoundingSphere : public Collider
	{
	public:
		BoundingSphere(const glm::vec3& center, float radius)
			: m_Center(center), m_Radius(radius)
		{}
		~BoundingSphere() {}

		virtual IntersectData IntersectRay(const Ray& ray) const override;
		virtual IntersectData IntersectBoundingSphere(const Ref<BoundingSphere>& other) const override;

		//Getting
		inline const glm::vec3& GetCenter() { return m_Center; }
		inline const float GetRadius() const { return m_Radius; }

	private:
		glm::vec3 m_Center;
		float m_Radius;
	};
}