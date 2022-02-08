#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	struct MeshDataBuffer
	{
		glm::vec4 albedoColor;
		glm::vec4 normalColor;

		glm::vec2 blendingUseBlending;
		glm::vec2 mroColor;

		uint32_t useAlbedo;
		uint32_t useNormal;
		uint32_t useMRO;
		uint32_t useSkybox;
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
		uint32_t xScreenTiles;
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

	struct LightCullingBuffer
	{
		alignas(16) glm::vec2 screenSize;
		uint32_t lightCount;
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

	struct TerrainDataBuffer
	{
		alignas(16) float heightMultiplier = 0.f;
	};

	struct HBAODataBuffer
	{
		glm::vec4 perspectiveInfo;
		glm::vec2 invQuarterResolution;

		float radiusToScreen;
		float negInvR2;
		float NdotVBias;
		float aoMultiplier;
		float powExponent;

		bool isOrtho;

		glm::vec4 float2Offsets[16];
		glm::vec4 jitters[16];
	};
}