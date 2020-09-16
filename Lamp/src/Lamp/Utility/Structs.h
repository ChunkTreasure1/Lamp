#pragma once

#include <glm/glm.hpp>

struct SAABB
{
	glm::vec3 Max = glm::vec3(0.f);
	glm::vec3 Min = glm::vec3(0.f);
};