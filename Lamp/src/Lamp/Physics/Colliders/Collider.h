#pragma once

#include "Lamp/Physics/Ray.h"
#include "Lamp/Core/Core.h"

namespace Lamp
{
	class BoundingSphere;
	class AABB;
	
	struct IntersectData
	{
		IntersectData(const bool intersecting, const float dist)
			: IsIntersecting(intersecting), Distance(dist)
		{}

		const bool IsIntersecting;
		const float Distance;
	};

	class Collider
	{
	public:
		Collider() {}
		~Collider() {}

		virtual IntersectData IntersectRay(const Ray& ray) const { return IntersectData(false, 0.f); }
		virtual IntersectData IntersectBoundingSphere(const Ref<BoundingSphere>& other) const { return IntersectData(false, 0.f); }
		virtual IntersectData IntersectAABB(const Ref<AABB>& other) const { return IntersectData(false, 0.f); }
	};
}