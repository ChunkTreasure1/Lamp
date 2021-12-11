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
	struct CameraRenderData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec4 positionAndTanHalfFOV;
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

	struct SSAODataBuffer
	{
		glm::vec4 kernelSamples[256];
		int kernelSize = 64;
		float bias = 0.025f;
		float radius = 0.1f;
		float strength = 1.f;
	};
}