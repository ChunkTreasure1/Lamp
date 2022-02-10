#include "lppch.h"
#include "BoundingVolume.h"

namespace Lamp
{
	BoundingSphere::BoundingSphere(const glm::vec3& aCenter, float aRadius)
		: center(aCenter), radius(aRadius)
	{
	}

	bool BoundingSphere::IsOnOrForwardPlan(const Plane& plane) const
	{
		return plane.GetSignedDistanceToPlane(center) > -radius;
	}

	bool BoundingSphere::IsInFrustum(const Frustum& frustum, const glm::mat4& transform) const
	{
		glm::vec3 globalScale = { glm::length(transform[0]), glm::length(transform[1]), glm::length(transform[2]) };
		const glm::vec3 globalCenter = transform * glm::vec4(center, 1.f);

		const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

		BoundingSphere globalSphere(globalCenter, radius * (maxScale * 0.5f));

		return (
			globalSphere.IsOnOrForwardPlan(frustum.leftFace) &&
			globalSphere.IsOnOrForwardPlan(frustum.rightFace) &&
			globalSphere.IsOnOrForwardPlan(frustum.farFace) &&
			globalSphere.IsOnOrForwardPlan(frustum.nearFace) &&
			globalSphere.IsOnOrForwardPlan(frustum.topFace) &&
			globalSphere.IsOnOrForwardPlan(frustum.bottomFace)
			);
	}
}