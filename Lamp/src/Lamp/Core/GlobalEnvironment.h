#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

#include "Lamp/Rendering/LightBase.h"

namespace Lamp
{
	class Level;
	class AssetManager;
	class Framebuffer;
}

struct GlobalEnvironment
{
	Lamp::DirectionalLight DirLight;
	bool ShouldRenderBB = false;
	bool ShouldRenderGizmos = true;
	bool IsEditor = false;
	float HDRExposure = 3.f;

	std::shared_ptr<Lamp::Framebuffer> pSkyboxBuffer = nullptr;
	std::shared_ptr<Lamp::Level> pLevel = nullptr;
	Lamp::AssetManager* pAssetManager = nullptr;
};