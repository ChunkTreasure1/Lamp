#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	struct Plane
	{
		Plane() = default;
		Plane(const glm::vec3& aPoint, const glm::vec3& aNormal)
			: normal(glm::normalize(aNormal)), point(aPoint)
		{ }

		float GetSignedDistanceToPlane(const glm::vec3& position) const
		{
			return glm::dot(position - point, normal);
		}

		glm::vec3 normal = { 0.f, 1.f, 0.f };
		glm::vec3 point = { 0.f, 0.f, 0.f };
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