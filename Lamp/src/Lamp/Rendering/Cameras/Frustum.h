#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	struct Plane
	{
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