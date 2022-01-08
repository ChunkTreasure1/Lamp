#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace Lamp
{
	static int s_lightId = 0;
	class PointShadowBuffer;
	class Framebuffer;
	class RenderPipeline;

	struct DirectionalLight
	{
		DirectionalLight();

		glm::vec3 color{ 1.f, 1.f, 1.f };
		float intensity = 1.f;
		bool castShadows = true;

		glm::mat4 viewProjection = glm::mat4(1.f);

		glm::mat4 transform = glm::mat4(1.f);

		uint32_t Id = s_lightId++;

		std::shared_ptr<Framebuffer> shadowBuffer;
		std::shared_ptr<RenderPipeline> shadowPipeline;
	};

	struct PointLight
	{
		PointLight();

		glm::vec3 color{ 1.f, 1.f, 1.f };

		float intensity = 1.f;
		float radius = 1.f;
		float falloff = 0.f;
		float farPlane = 100.f;
		float nearPlane = 0.01f;

		uint32_t id = s_lightId++;
		std::shared_ptr<PointShadowBuffer> shadowBuffer;
	};
}