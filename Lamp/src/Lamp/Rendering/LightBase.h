#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lamp
{
	struct DirectionalLight
	{
		DirectionalLight()
		{
			ViewProjection = Projection * View;
		}

		glm::vec3 Position{ 0.f, 10.f, -10.f };
		glm::vec3 Direction = glm::vec3(0.f) - Position;

		glm::mat4 ViewProjection = glm::mat4(1.f);

		glm::vec3 Color{ 1.f, 1.f, 1.f };
		float Intensity = 1.f;
		float Bias = 0.5f;

	private:
		glm::mat4 Projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 100.f);
		glm::mat4 View = glm::lookAt(Position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

		inline void SetPosition(const glm::vec3& pos)
		{
			Position = pos; 
			Direction = glm::vec3(0.f) - Position;
			Direction = glm::normalize(Direction);

			View = glm::lookAt(Position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
			ViewProjection = Projection * View;
		}
	};

	struct PointLight
	{
		glm::vec3 Color{ 1.f, 1.f, 1.f };

		float Intensity = 1.f;
		float Radius = 1.f;
		float Falloff = 0.f;
	};
}