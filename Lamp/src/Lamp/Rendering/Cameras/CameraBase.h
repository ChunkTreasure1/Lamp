#pragma once

#include <glm/glm.hpp>

namespace Lamp
{
	class CameraBase
	{
	public:
		CameraBase() 
			: m_ProjectionMatrix(glm::mat4(1.f)), m_ViewMatrix(glm::mat4(1.f)), m_ViewProjectionMatrix(glm::mat4(1.f))
		{}
		virtual ~CameraBase() {}

		virtual void SetProjection(float x, float y, float z, float k) {}

		//Getting
		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		inline const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		inline const glm::vec3& GetRotation() const { return m_Rotation; }

		//Setting
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; RecalculateViewMatrix(); }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; RecalculateViewMatrix(); }

	protected:
		virtual void RecalculateViewMatrix() {}

	protected:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position = { 0.f, 0.f, 0.f };
		glm::vec3 m_Rotation = { 0.f, 0.f, 0.f };
	};
}