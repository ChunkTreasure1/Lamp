#include "lppch.h"
#include "PerspectiveCamera.h"
#include "Lamp/Core/Application.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


namespace Lamp
{
	PerspectiveCamera::PerspectiveCamera(float fov, float nearPlane, float farPlane)
		: m_TransformMatrix(1.f), m_fieldOfView(fov), m_nearPlane(nearPlane), m_farPlane(farPlane)
	{
		m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), (float)Application::Get().GetWindow().GetWidth() / Application::Get().GetWindow().GetHeight(), nearPlane, farPlane);
		m_viewMatrix = glm::mat4(1.f);
		m_aspectRatio = (float)Application::Get().GetWindow().GetWidth() / (float)Application::Get().GetWindow().GetHeight();

		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	}

	void PerspectiveCamera::SetProjection(float fov, float aspect, float nearPlane, float farPlane)
	{
		m_fieldOfView = fov;
		m_aspectRatio = aspect;

		m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), aspect, nearPlane, farPlane);
		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	}

	Frustum PerspectiveCamera::CreateFrustum()
	{
		Frustum frustum;
		const float halfVSide = m_farPlane * tanf(glm::radians(m_fieldOfView) * 0.5f);
		const float halfHSide = halfVSide * m_aspectRatio;

		const glm::vec3 forwardDir = glm::normalize(GetForwardDirection());
		const glm::vec3 upDir = glm::normalize(GetUpDirection());
		const glm::vec3 rightDir = glm::normalize(GetRightDirection());

		const glm::vec3 frontMultFar = m_farPlane * forwardDir;

		frustum.nearFace = { m_position + m_nearPlane * forwardDir, forwardDir };
		frustum.farFace = { m_position + frontMultFar, -forwardDir };

		frustum.rightFace = { m_position, glm::cross(upDir, frontMultFar + rightDir * halfHSide) };
		frustum.leftFace = { m_position, glm::cross(frontMultFar - rightDir * halfHSide, upDir) };

		frustum.topFace = { m_position, glm::cross(rightDir, frontMultFar - upDir * halfVSide) };
		frustum.bottomFace = { m_position, glm::cross(frontMultFar + upDir * halfVSide, rightDir) };

		return frustum;
	}

	void PerspectiveCamera::RecalculateViewMatrix()
	{
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

		const glm::vec3 lookAt = m_position + GetForwardDirection();
		m_worldRotation = glm::normalize((m_position - GetForwardDirection()) - m_position);
		m_viewMatrix = glm::lookAt(m_position, lookAt, glm::vec3(0.f, yawSign, 0.f));

		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
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
		return glm::quat(glm::vec3(glm::radians(-m_rotation.y), glm::radians(-m_rotation.x), 0.f));
	}

	void PerspectiveCamera::UpdateVectors()
	{
		const float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Right = glm::cross(m_worldRotation, glm::vec3(0.f, yawSign, 0.f));

		RecalculateViewMatrix();
	}
}