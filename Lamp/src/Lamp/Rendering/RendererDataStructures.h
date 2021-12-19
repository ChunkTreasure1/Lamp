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

	struct CubeBuffer
	{
		alignas(16) glm::mat4 modelViewProjection;
		alignas(16) glm::vec2 phiTheta{ (2.f * glm::pi<float>()) / 180.f, (2.f * glm::pi<float>()) / 64.f };
	};

	struct ScreenDataBuffer
	{
		alignas(16) glm::vec2 size;
		alignas(16) float aspectRatio;
	};

	struct DirectionalLightData
	{
		alignas(16) glm::vec4 direction;
		alignas(16) glm::vec4 colorIntensity;
		alignas(16) bool castShadows;
	};

	struct DirectionalLightDataBuffer
	{
		alignas(16) DirectionalLightData dirLights[1];
		alignas(16) uint32_t lightCount = 0;
	};

	struct SSAODataBuffer
	{
		alignas(16) glm::vec4 kernelSamples[256];
		alignas(16) glm::vec4 sizeBiasRadiusStrength{ 64.f, 0.025f, 0.1f, 1.f };
	};
}