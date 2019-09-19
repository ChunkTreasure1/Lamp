#include "lppch.h"
#include "Camera2D.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Lamp
{
	Camera2D::Camera2D(uint32_t width, uint32_t height)
		: m_Position(0.f, 0.f), m_CameraMatrix(1.f),
		m_OrthoMatrix(1.f), m_Scale(1.f),
		m_MatrixChanged(true),
		m_WindowHeight(height), m_WindowWidth(width)
	{}

	Camera2D::~Camera2D()
	{}

	void Camera2D::Update()
	{
		if (m_MatrixChanged)
		{
			//Move the camera
			glm::vec3 translate(-m_Position.x + m_WindowWidth / 2, -m_Position.y + m_WindowHeight / 2, 0.f);
			m_CameraMatrix = glm::translate(m_OrthoMatrix, translate);

			glm::vec3 scale(m_Scale, m_Scale, 0.f);
			m_CameraMatrix = glm::scale(glm::mat4(1.f), scale) * m_CameraMatrix;

			m_MatrixChanged = false;
		}
	}

	void Camera2D::Initialize(uint32_t width, uint32_t height)
	{
		//Set the start ortho matrix
		m_OrthoMatrix = glm::ortho(0.f, (float)m_WindowWidth, 0.f, (float)m_WindowHeight);
	}

	glm::vec2 Camera2D::ScreenToWorldCoords(glm::vec2 screenCoords)
	{
		//Invert Y
		screenCoords.y = m_WindowHeight - screenCoords.y;

		//Change the origo pos and fix scaling
		screenCoords -= glm::vec2(m_WindowWidth / 2, m_WindowHeight / 2);
		screenCoords /= m_Scale;

		screenCoords += m_Position;

		return screenCoords;
	}
}