#pragma once

namespace Lamp
{
	//Lights data structs
	struct PointLightData
	{
		glm::vec4 position;
		glm::vec4 color;
		float intensity;
		float radius;
		float falloff;
		float farPlane;

		int samplerId;
		int padding[3];
	};
		
	//Uniform buffers

	struct DirectionalLightVPs
	{
		glm::mat4 viewProjections[10];
		uint32_t lightCount = 0;
	};

	struct LightIndex
	{
		int index;
	};


}