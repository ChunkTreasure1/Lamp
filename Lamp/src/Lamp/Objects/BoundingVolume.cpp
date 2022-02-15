#include "lppch.h"
#include "BoundingVolume.h"

#include "Lamp/AssetSystem/ResourceCache.h"
#include "Lamp/Mesh/Mesh.h"

namespace Lamp
{
	BoundingSphere::BoundingSphere(const glm::vec3& aCenter, float aRadius)
		: center(aCenter), radius(aRadius)
	{
	}

	bool BoundingSphere::IsOnOrForwardPlan(const Plane& plane) const
	{
		float distance = plane.GetSignedDistanceToPlane(center);
		return distance > -radius;
	}

	bool BoundingSphere::IsInFrustum(const Frustum& frustum, const glm::mat4& transform) const
	{
		glm::vec3 globalScale = { glm::length(transform[0]), glm::length(transform[1]), glm::length(transform[2]) };
		const glm::vec3 globalCenter = transform * glm::vec4(center, 1.f);

		const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

		BoundingSphere globalSphere(globalCenter, radius * maxScale * 0.5f);

		bool l = globalSphere.IsOnOrForwardPlan(frustum.leftFace);
		bool r = globalSphere.IsOnOrForwardPlan(frustum.rightFace);
		bool f = globalSphere.IsOnOrForwardPlan(frustum.farFace);
		bool n = globalSphere.IsOnOrForwardPlan(frustum.nearFace);
		bool t = globalSphere.IsOnOrForwardPlan(frustum.topFace);
		bool b = globalSphere.IsOnOrForwardPlan(frustum.bottomFace);
	
		return (l && r && f && n && t && b);
	}
}