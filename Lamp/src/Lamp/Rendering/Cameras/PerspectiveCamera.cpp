#include "lppch.h"
#include "PerspectiveCamera.h"
#include "Lamp/Core/Application.h"

namespace Lamp
{
	PerspectiveCamera::PerspectiveCamera(float fov, float nearPlane, float farPlane)
		: m_ProjectionMatrix(glm::perspective(glm::radians(fov), (float)Application::Get().GetWindow().GetWidth() / Application::Get().GetWindow().GetHeight(), nearPlane, farPlane)), m_ViewMatrix(1.f)
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::SetProjection(float fov, float aspect, float nearPlane, float farPlane)
	{
		m_ProjectionMatrix = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.f), m_Position)
			* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.x), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.y), glm::vec3(0.f, 1.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(m_Rotation.z), glm::vec3(0.f, 0.f, 1.f));

		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}