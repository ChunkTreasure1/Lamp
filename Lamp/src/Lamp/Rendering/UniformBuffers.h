#pragma once

namespace Lamp
{
	//Lights data structs
	struct DirectionalLightData
	{
		glm::vec4 direction;
		glm::vec4 color;
		float intensity;
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
	struct CommonBuffer
	{
		glm::mat4 View;
		glm::mat4 Projection;
		glm::mat4 ShadowVP;
		glm::vec4 CameraPosition;
	};

	struct DirectionalLightBuffer
	{
		DirectionalLightData dirLight;
	};

	struct LightIndex
	{
		int index;
	};
}