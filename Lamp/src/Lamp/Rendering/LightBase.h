#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lamp
{
	static int s_PointLightId = 0;
	class PointShadowBuffer;

	struct DirectionalLight
	{
		DirectionalLight()
		{
			ViewProjection = Projection * View;
		}

		glm::vec3 Position{ 50.f, 50.f, 0.f };

		glm::mat4 ViewProjection = glm::mat4(1.f);

		glm::vec3 Color{ 1.f, 1.f, 1.f };
		float Intensity = 1.f;

		void UpdateProjection()
		{
			View = glm::lookAt(Position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
			ViewProjection = Projection * View;
		}

	private:
		glm::mat4 Projection = glm::ortho(-25.f, 25.f, -25.f, 25.f, 0.1f, 1000.f);
		glm::mat4 View = glm::lookAt(Position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

		inline void SetPosition(const glm::vec3& pos)
		{
			Position = pos; 

			View = glm::lookAt(Position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
			ViewProjection = Projection * View;
		}
	};

	struct PointLight
	{
		PointLight()
		{
			Id = s_PointLightId++;
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