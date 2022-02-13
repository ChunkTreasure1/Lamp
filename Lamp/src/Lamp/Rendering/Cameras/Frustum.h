#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	struct Plane
	{
		Plane() = default;
		Plane(const glm::vec3& aNormal, float aDistance)
			: normal(glm::normalize(aNormal)), distance(aDistance)
		{ }

		Plane(const glm::vec3& point, const glm::vec3& aNormal)
			: normal(glm::normalize(aNormal)), distance(glm::dot(aNormal, point))
		{ }

		float GetSignedDistanceToPlane(const glm::vec3& point) const
		{
			return glm::dot(normal, point) - distance;
		}

		glm::vec3 normal = { 0.f, 1.f, 0.f };
		float distance = 0.f;
	};

	struct Frustum
	{
		Plane topFace;
		Plane bottomFace;
		
		Plane rightFace;
		Plane leftFace;

		Plane farFace;
		Plane nearFace;
	};
}