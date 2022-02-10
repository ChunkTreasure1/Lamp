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

		void SetProjection(float fov, float aspect, float nearPlane, float farPlane) override;
		Frustum CreateFrustum() override;

		//Getting
		inline const float GetYaw() const { return m_rotation.x; }
		inline const float GetPitch() const { return m_rotation.y; }
		inline const glm::mat4& GetTM() { return m_TransformMatrix; }
		inline float& GetFieldOfView() { return m_fieldOfView; }
		inline float GetAspectRatio() const { return m_aspectRatio; }

		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;

		//Setting
		inline void SetYaw(float yaw) { m_rotation.x = yaw; }
		inline void SetPitch(float pitch) { m_rotation.y = pitch; }
		inline void SetFront(const glm::vec3& front) { m_Front = front; }

		void UpdateVectors();

	private:
		void RecalculateViewMatrix() override;
		glm::quat GetOrientation() const;

	private:
		glm::mat4 m_TransformMatrix;

		glm::vec3 m_WorldUp = { 0.f, 1.f, 0.f };
		glm::vec3 m_Front = { 0.f, 0.f, -1.f };
		glm::vec3 m_Up = { 0.f, 1.f, 0.f };
		glm::vec3 m_Right = { -1.f, 0.f, 0.f };

		glm::vec3 m_worldRotation = { 0.f, 0.f, 0.f };

		float m_fieldOfView;
		float m_aspectRatio;
		float m_farPlane;
		float m_nearPlane;
		bool m_FirstTime = false;
	};
}