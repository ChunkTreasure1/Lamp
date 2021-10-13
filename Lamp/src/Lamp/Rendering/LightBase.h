#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace Lamp
{
	static int s_LightId = 0;
	class PointShadowBuffer;

	struct DirectionalLight
	{
		DirectionalLight()
		{
			ViewProjection = m_Projection * m_View;
			Id = s_LightId++;
		}

		glm::vec3 Direction{ 50.f, 50.f, 0.f };

		glm::mat4 ViewProjection = glm::mat4(1.f);

		glm::vec3 Color{ 1.f, 1.f, 1.f };
		float Intensity = 1.f;

		void UpdateView()
		{
			m_View = glm::lookAt(Direction, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
			ViewProjection = m_Projection * m_View;
		}

		void UpdateProjection(const glm::vec3& pos)
		{
			m_Projection = glm::ortho(-m_Size.x * pos.x, m_Size.x * pos.x, -m_Size.y * pos.z, m_Size.y * pos.z, 0.1f, 1000.f);
			ViewProjection = m_Projection * m_View;
		}

		uint32_t Id;

	private:
		glm::mat4 m_Projection = glm::ortho(-25.f, 25.f, -25.f, 25.f, 0.1f, 1000.f);
		glm::mat4 m_View = glm::lookAt(Direction, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));

		glm::vec2 m_Size = { 25.f, 25.f };

		inline void SetPosition(const glm::vec3& pos)
		{
			Direction = pos;

			m_View = glm::lookAt(Direction, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
			ViewProjection = m_Projection * m_View;
		}
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