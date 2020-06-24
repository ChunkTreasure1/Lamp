#pragma once

#include <glm/glm.hpp>
#include "Collider.h"

namespace Lamp
{
	class Plane : public Collider
	{
	public:
		Plane(const glm::vec3& normal, const glm::vec3& center)
			: Collider(CollType::Plane), m_Normal(normal), m_Center(center)
		{}

		Plane Normalized() const;
		virtual IntersectData IntersectRay(const Ray& ray) const override;
		virtual IntersectData Intersect(const Ref<Collider>& other) const override;
		virtual void Transform(const glm::vec3& translation) override;
		virtual void SetTranslation(const glm::vec3& trans) { m_Normal = trans; }

		//Getting
		inline const glm::vec3& GetNormal() const { return m_Normal; }
		inline const glm::vec3& GetCenter() const { return m_Center; }

	private:
		glm::vec3 m_Normal;
		glm::vec3 m_Center;
	};
}