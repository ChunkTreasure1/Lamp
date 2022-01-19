#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	struct MeshDataBuffer
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::vec2 blendingUseBlending;
	};

	struct CameraDataBuffer
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 projection;
		alignas(16) glm::vec4 positionAndTanHalfFOV;
		alignas(16) glm::vec2 ambienceExposure;
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

	struct DirectionalLightVPBuffer
	{
		alignas(16) glm::mat4 directionalLightVPs[10];
		alignas(16) uint32_t count;
	};

	struct SSAODataBuffer
	{
		alignas(16) glm::vec4 kernelSamples[256];
		alignas(16) glm::vec4 sizeBiasRadiusStrength{ 64.f, 0.025f, 0.1f, 1.f };
	};

	struct LightCullingBuffer
	{
		alignas(16) glm::vec2 screenSize;
		alignas(16) uint32_t lightCount;
	};

	struct PointLightData
	{
		alignas(16) glm::vec4 position;
		alignas(16) glm::vec4 color;

		float intensity;
		float radius;
		float falloff;
		float farPlane;

		alignas(16) int samplerId;
	};

	struct LightIndex
	{
		int index;
	};
}