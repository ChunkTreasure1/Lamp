#pragma once

namespace Lamp
{
	//Lights data structs
	struct DirectionalLightData
	{
		glm::vec3 direction;
		float intensity;
		glm::vec3 color;
		float padding;
	};

	struct PointLightData
	{
		glm::vec3 position;
		float radius;
		glm::vec3 color;
		float intensity;
	};

	//Uniform buffers
	struct CommonBuffer
	{
		glm::mat4 View;
		glm::mat4 Projection;
		glm::mat4 ShadowVP;
		glm::vec3 CameraPosition;
	};

	struct LightBuffer
	{
		DirectionalLightData dirLight;
		PointLightData pointLights[1];
		uint32_t lightCount;
	};
}