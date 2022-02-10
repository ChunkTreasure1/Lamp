#pragma once

#include "Lamp/Rendering/Cameras/Frustum.h"

namespace Lamp
{
	struct BoundingVolume
	{
		virtual bool IsInFrustum(const Frustum& frustum, const glm::mat4& transform) const = 0;
	};

	struct BoundingSphere : BoundingVolume
	{
		BoundingSphere(const glm::vec3& aCenter, float aRadius);

		bool IsOnOrForwardPlan(const Plane& plan) const;
		bool IsInFrustum(const Frustum& frustum, const glm::mat4& transform) const override;

		glm::vec3 center{ 0.f, 0.f, 0.f };
		float radius = 0.f;
	};
}