#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "CameraBase.h"

namespace Lamp
{
	class PerspectiveCamera : public CameraBase
	{
	public:
		PerspectiveCamera(float fov, float nearPlane, float farPlane);

		virtual void SetProjection(float fov, float aspect, float nearPlane, float farPlane) override;

		//Getting
		inline const glm::vec3& GetFront() const { return m_Front; }
		inline const glm::vec3& GetUp() const { return m_Up; }
		inline const glm::vec3& GetRight() const { return m_Right; }

		inline const float GetYaw() const { return m_Yaw; }
		inline const float GetPitch() const { return m_Pitch; }
		inline const glm::mat4& GetTM() { return m_TransformMatrix; }

		//Setting
		inline void SetYaw(float yaw) { m_Yaw = yaw; }
		inline void SetPitch(float pitch) { m_Pitch = pitch; }
		inline void SetFront(const glm::vec3& front) { m_Front = front; }

		void UpdateVectors();

	private:
		virtual void RecalculateViewMatrix() override;

	private:
		glm::mat4 m_TransformMatrix;

		glm::vec3 m_WorldUp = { 0.f, 1.f, 0.f };
		glm::vec3 m_Front = { 0.f, 0.f, -1.f };
		glm::vec3 m_Up = { 0.f, 1.f, 0.f };
		glm::vec3 m_Right = { 1.f, 0.f, 0.f };

		float m_Yaw = -90.0f;;
		float m_Pitch = 0.f;
	};
}