#include "lppch.h"
#include "OrthographicCameraController.h"

#include "Lamp/Input/Input.h"
#include "Lamp/Input/KeyCodes.h"

namespace Lamp
{
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation),
		m_HasControl(true)
	{
	}

	void OrthographicCameraController::Update(Timestep ts)
	{
		if (m_HasControl)
		{
			if (Input::IsKeyPressed(LP_KEY_A))
				m_CameraPosition.x -= m_CameraTranslationSpeed * ts;
			else if (Input::IsKeyPressed(LP_KEY_D))
				m_CameraPosition.x += m_CameraTranslationSpeed * ts;

			if (Input::IsKeyPressed(LP_KEY_W))
				m_CameraPosition.y += m_CameraTranslationSpeed * ts;
			else if (Input::IsKeyPressed(LP_KEY_S))
				m_CameraPosition.y -= m_CameraTranslationSpeed * ts;

			if (m_Rotation)
			{
				if (Input::IsKeyPressed(LP_KEY_Q))
					m_CameraRotation += m_CameraRotationSpeed * ts;
				if (Input::IsKeyPressed(LP_KEY_E))
					m_CameraRotation -= m_CameraRotationSpeed * ts;

				m_Camera.SetRotation(m_CameraRotation);
			}

			m_Camera.SetPosition(m_CameraPosition);

			m_CameraTranslationSpeed = m_ZoomLevel;
		}
	}

	void OrthographicCameraController::OnEvent(Event & e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(LP_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent & e)
	{
		if (m_HasControl)
		{
			m_ZoomLevel -= e.GetYOffset() * 0.25f;
			m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
			m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
			return false;
		}
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent & e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}
}