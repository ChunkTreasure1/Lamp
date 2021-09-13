#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

#include "Lamp/Rendering/LightBase.h"

namespace Lamp
{
	class Level;
	class AssetManager;
}

struct GlobalEnvironment
{
	Lamp::DirectionalLight DirLight;
	bool ShouldRenderBB = false;
	bool ShouldRenderGizmos = true;
	bool IsEditor = false;
	float HDRExposure = 3.f;

	std::shared_ptr<Lamp::Level> pLevel = nullptr;
	Lamp::AssetManager* pAssetManager = nullptr;
};

struct SAABB
{
	glm::vec3 Max = glm::vec3(0.f);
	glm::vec3 Min = glm::vec3(0.f);

	glm::vec3 StartMax = glm::vec3(0.f);
	glm::vec3 StartMin = glm::vec3(0.f);
};