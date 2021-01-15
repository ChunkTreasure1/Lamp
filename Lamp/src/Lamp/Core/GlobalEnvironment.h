#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Lamp/Rendering/LightBase.h"

struct GlobalEnvironment
{
	Lamp::DirectionalLight DirLight;
	bool ShouldRenderBB = false;
};

struct SAABB
{
	glm::vec3 Max = glm::vec3(0.f);
	glm::vec3 Min = glm::vec3(0.f);

	glm::vec3 StartMax = glm::vec3(0.f);
	glm::vec3 StartMin = glm::vec3(0.f);
};