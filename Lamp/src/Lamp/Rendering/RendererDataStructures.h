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
		alignas(16) glm::vec4 positionAndTanHalfFOV;
	};

	struct ScreenDataBuffer
	{
		alignas(16) glm::vec2 size;
		float aspectRatio;
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

	struct SSAODataBuffer
	{
		glm::vec4 kernelSamples[256];
		int kernelSize = 64;
		float bias = 0.025f;
		float radius = 0.1f;
		float strength = 1.f;
	};
}