#pragma once

#include "Lamp/Physics/Ray.h"
#include "Lamp/Core/Core.h"

namespace Lamp
{
	class BoundingSphere;
	class AABB;
	
	struct IntersectData
	{
		IntersectData(const bool intersecting, const glm::vec3 dir)
			: IsIntersecting(intersecting), Direction(dir)
		{}

		const bool IsIntersecting;
		const glm::vec3 Direction;
	};

	enum class CollType
	{
		None = 1,
		Sphere,
		AABB,
		Plane
	};

	class Collider
	{
	public:
		Collider(CollType type)
			: m_Type(type)
		{}
		~Collider() {}

		virtual IntersectData IntersectRay(const Ray& ray) const = 0;
		virtual IntersectData Intersect(const Ref<Collider>& other) const = 0;
		virtual void Transform(const glm::vec3& translation) = 0;
		virtual void SetTranslation(const glm::vec3& trans) = 0;

		//Getting
		inline CollType GetType() const { return m_Type; }

	private:
		CollType m_Type = CollType::None;
	};
}