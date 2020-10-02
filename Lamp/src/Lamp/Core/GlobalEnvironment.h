#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Lamp/Rendering/LightBase.h"

struct LevelEnvironment
{
	glm::vec3 GlobalAmbient{ 0.3f, 0.3f, 0.3f };
};

struct GlobalEnvironment
{
	LevelEnvironment GlobalLevelEnvironment;

	Lamp::DirectionalLight DirLight;
	bool ShouldRenderBB = false;
};

struct SAABB
{
	glm::vec3 Max = glm::vec3(0.f);
	glm::vec3 Min = glm::vec3(0.f);
};