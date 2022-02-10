#pragma once

#include "Frustum.h"

#include <glm/glm.hpp>

namespace Lamp
{
	class CameraBase
	{
	public:
		CameraBase() 
			: m_projectionMatrix(glm::mat4(1.f)), m_viewMatrix(glm::mat4(1.f)), m_viewProjectionMatrix(glm::mat4(1.f))
		{}
		virtual ~CameraBase() {}

		virtual void SetProjection(float x, float y, float z, float k) {}
		virtual Frustum CreateFrustum() = 0;

		inline const glm::vec3& GetPosition() const { return m_position; }
		inline const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
		inline const glm::mat4& GetViewProjectionMatrix() const { return m_viewProjectionMatrix; }
		inline const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
		inline const glm::vec3& GetRotation() const { return m_rotation; }

		inline void SetPosition(const glm::vec3& pos) { m_position = pos; RecalculateViewMatrix(); }
		inline void SetRotation(const glm::vec3& rot) { m_rotation = rot; RecalculateViewMatrix(); }

	protected:
		virtual void RecalculateViewMatrix() {}

	protected:
		glm::mat4 m_projectionMatrix;
		glm::mat4 m_viewMatrix;
		glm::mat4 m_viewProjectionMatrix;

		glm::vec3 m_position = { 0.f, 0.f, 0.f };
		glm::vec3 m_rotation = { 0.f, 0.f, 0.f };
	};
}