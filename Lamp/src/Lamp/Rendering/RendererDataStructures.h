#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	struct MeshDataBuffer
	{
		alignas(16) glm::mat4 model;
	};

	struct CameraDataBuffer
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
		alignas(16) glm::vec4 position;
	};

	struct DirectionalLightData
	{
		alignas(16) glm::vec4 direction;
		alignas(16) glm::vec4 colorIntensity;
		alignas(16) bool castShadows;
	};

	struct DirectionalLightDataBuffer
	{
		DirectionalLightData dirLights[1];
		uint32_t lightCount = 0;
	};
}