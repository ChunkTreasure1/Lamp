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
	bool shouldRenderBB = false;
	bool shouldRenderGizmos = true;
	bool isEditor = false;
	float hdrExposure = 3.f;

	std::shared_ptr<Lamp::Level> pLevel = nullptr;
	Lamp::AssetManager* pAssetManager = nullptr;
};