#include "Camera2D.h"

namespace Lamp
{
	Camera2D::Camera2D(WindowProps& props)
		: m_Position(0.f, 0.f),
		m_CameraMatrix(1.0f),
		m_OrthoMatrix(1.0f),
		m_Scale(1.0f),
		m_MatrixChanged(true)
	{
		//Set the window size
		m_WindowWidth = props.Width;
		m_WindowHeight = props.Height;

		//Set the orthographic matrix
		m_OrthoMatrix = glm::ortho(0.0f, (float)m_WindowWidth, 0.0f, (float)m_WindowHeight);
	}

	Camera2D::~Camera2D()
	{}

	//Updates the camera	
	void Camera2D::Update()
	{
		//Check if the transform matrix has changed
		if (m_MatrixChanged)
		{
			//Move the camera using translation
			glm::vec3 translate(-m_Position.x + m_WindowWidth / 2, -m_Position.y + m_WindowHeight / 2, 0.0f);
			m_CameraMatrix = glm::translate(m_OrthoMatrix, translate);

			//Scale the camera
			glm::vec3 scale(m_Scale, m_Scale, 0.0f);
			m_CameraMatrix = glm::scale(glm::mat4(1.0f), scale) * m_CameraMatrix;

			m_MatrixChanged = false;
		}
	}

	//Converts screen coords to world coords
	glm::vec2 Camera2D::ScreenToWorldCoords(glm::vec2 screenCoords)
	{
		//Invert Y
		screenCoords.y = m_WindowHeight - screenCoords.y;

		//Change the origo pos and fix scaling
		screenCoords -= glm::vec2(m_WindowWidth / 2, m_WindowHeight / 2);
		screenCoords /= m_Scale;

		//Translate with the camera
		screenCoords += m_Position;

		return screenCoords;
	}
}
