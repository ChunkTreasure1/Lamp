#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;
		float t;
	};
}