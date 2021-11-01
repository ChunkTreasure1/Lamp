#pragma once

namespace Lamp
{
	//Lights data structs
	struct DirectionalLightData
	{
		glm::vec4 direction;
		glm::vec4 colorIntensity;
	};

	struct PointLightData
	{
		glm::vec4 position;
		glm::vec4 color;
		float intensity;
		float radius;
		float falloff;
		float farPlane;
	};

	//Uniform buffers
	struct CommonRenderData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 cameraPosition;
	};

	struct DirectionalLightBuffer
	{
		DirectionalLightData dirLights[10];
		uint32_t lightCount = 0;
	};

	struct DirectionalLightVPs
	{
		glm::mat4 viewProjections[10];
		uint32_t lightCount = 0;
	};

	struct LightIndex
	{
		int index;
	};

	struct SSAOData
	{
		glm::vec4 kernelSamples[256];
		uint32_t kernelSize = 64;
		float bias = 0.025f;
		float radius = 0.5f;
		float t = 0.f;
	};
}