#include "lppch.h"
#include "PerspectiveCamera.h"
#include "Lamp/Core/Application.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


namespace Lamp
{
	PerspectiveCamera::PerspectiveCamera(float fov, float nearPlane, float farPlane)
		: m_TransformMatrix(1.f), m_fieldOfView(fov)
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(m_fieldOfView), (float)Application::Get().GetWindow().GetWidth() / Application::Get().GetWindow().GetHeight(), nearPlane, farPlane);
		m_ViewMatrix = glm::mat4(1.f);
		m_aspectRatio = (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight();

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::SetProjection(float fov, float aspect, float nearPlane, float farPlane)
	{
		m_fieldOfView = fov;
		m_aspectRatio = aspect;

		m_ProjectionMatrix = glm::perspective(glm::radians(m_fieldOfView), aspect, nearPlane, farPlane);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

		const glm::vec3 lookAt = m_Position + GetForwardDirection();
		m_worldRotation = glm::normalize((m_Position - GetForwardDirection()) - m_Position);
		m_ViewMatrix = glm::lookAt(m_Position, lookAt, glm::vec3(0.f, yawSign, 0.f));

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	glm::vec3 PerspectiveCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.f, 1.f, 0.f));
	}

	glm::vec3 PerspectiveCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.f, 0.f, 0.f));
	}

	glm::vec3 PerspectiveCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.f, 0.f, -1.f));
	}

	glm::quat PerspectiveCamera::GetOrientation() const
	{
		return glm::quat(glm::vec3(glm::radians(-m_Rotation.y), glm::radians(-m_Rotation.x), 0.f));
	}

	void PerspectiveCamera::UpdateVectors()
	{
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Right = glm::cross(m_worldRotation, glm::vec3(0.f, yawSign, 0.f));

		RecalculateViewMatrix();
	}
}