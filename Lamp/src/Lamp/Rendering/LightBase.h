#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace Lamp
{
	static int s_LightId = 0;
	class PointShadowBuffer;
	class Framebuffer;
	class RenderPass;

	struct DirectionalLight
	{
		DirectionalLight();

		glm::vec3 Direction{ 50.f, 50.f, 0.f };
		glm::vec3 Color{ 1.f, 1.f, 1.f };
		float Intensity = 1.f;
		bool CastShadows = true;

		uint32_t Id = s_LightId++;
		std::unique_ptr<RenderPass> ShadowPass;
		std::shared_ptr<Framebuffer> ShadowBuffer;
	};

	struct PointLight
	{
		PointLight()
		{
			Id = s_LightId++;
		}

		glm::vec3 Color{ 1.f, 1.f, 1.f };

		float Intensity = 1.f;
		float Radius = 1.f;
		float Falloff = 0.f;
		float FarPlane = 100.f;
		float NearPlane = 0.01f;

		uint32_t Id;
		std::shared_ptr<PointShadowBuffer> ShadowBuffer;
	};
}