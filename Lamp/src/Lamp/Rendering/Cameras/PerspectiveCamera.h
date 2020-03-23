#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Lamp
{
	class PerspectiveCamera
	{
	public:
		PerspectiveCamera(float fov, float nearPlane, float farPlane);

		void SetProjection(float fov, float aspect, float nearPlane, float farPlane);

		//Getting
		inline const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		inline const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		inline const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		inline const glm::vec3& GetPosition() const { return m_Position; }
		inline const glm::vec3& GetRotation() const { return m_Rotation; }
		inline const glm::vec3& GetFront() const { return m_Front; }
		inline const glm::vec3& GetUp() const { return m_Up; }

		inline const float GetYaw() const { return m_Yaw; }
		inline const float GetPitch() const { return m_Pitch; }

		//Setting
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; RecalculateViewMatrix(); }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; RecalculateViewMatrix(); }
		inline void SetYaw(float yaw) { m_Yaw = yaw; }
		inline void SetPitch(float pitch) { m_Pitch = pitch; }

		inline void SetFront(const glm::vec3& front) { m_Front = front; }

	private:
		void RecalculateViewMatrix();

	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;

		glm::vec3 m_Front = { 0.f, 0.f, -1.f };
		glm::vec3 m_Up = { 0.f, 1.f, 0.f };

		float m_Yaw = 0.f;
		float m_Pitch = 0.f;
	};
}