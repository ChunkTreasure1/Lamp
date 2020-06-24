#include "lppch.h"
#include "OrthographicCameraController.h"

#include "Lamp/Input/Input.h"
#include "Lamp/Input/KeyCodes.h"

#include "Lamp/Core/Application.h"

namespace Lamp
{
	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_Rotation(rotation), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
	{
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}
	OrthographicCameraController::~OrthographicCameraController()
	{
	}

	void OrthographicCameraController::Update(Timestep ts)
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
		m_CameraTranslationSpeed = m_ZoomLevel;
		m_Camera.SetPosition(m_CameraPosition);

	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(LP_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(LP_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	glm::vec2 OrthographicCameraController::ScreenToWorldCoords(const glm::vec2& coords, const glm::vec2& windowSize)
	{
		float x = (coords.x / windowSize.x) * 2.f - 1;
		float y = (coords.y / windowSize.y) * 2.f - 1;
		float z = 0;

		glm::mat4 matInv = glm::inverse(m_Camera.GetViewProjectionMatrix());
		glm::vec4 coord = matInv * glm::vec4(x, -y, z, 1);

		return { coord.x, coord.y };
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.4f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return true;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return true;
	}
}