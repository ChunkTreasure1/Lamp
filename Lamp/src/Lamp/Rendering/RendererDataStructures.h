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
		alignas(16) glm::vec4 position;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
	};

	struct DirectionalLightDataTest
	{
		alignas(16) glm::vec4 direction;
		alignas(16) glm::vec4 colorIntensity;
	};
}