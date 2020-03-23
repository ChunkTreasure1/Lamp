#include "lppch.h"
#include "PerspectiveCameraController.h"
#include "Lamp/Input/Input.h"
#include "Lamp/Input/KeyCodes.h"

namespace Lamp
{
	PerspectiveCameraController::PerspectiveCameraController(float fov, float nearPlane, float farPlane)
		: m_Camera(fov, nearPlane, farPlane), m_FOV(fov), m_NearPlane(nearPlane), m_FarPlane(farPlane)
	{
		m_Camera.SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));
		m_Camera.SetRotation(glm::vec3(0.f, 0.f, 0.f));
	}

	PerspectiveCameraController::~PerspectiveCameraController()
	{
	}

	void PerspectiveCameraController::Update(Timestep ts)
	{
		if (Input::IsKeyPressed(LP_KEY_W))
		{
			m_CameraPosition += m_CameraTranslationSpeed * m_Camera.GetFront() * (float)ts;
		}
		if (Input::IsKeyPressed(LP_KEY_S))
		{
			m_CameraPosition -= m_CameraTranslationSpeed * m_Camera.GetFront() * (float)ts;
		}
		if (Input::IsKeyPressed(LP_KEY_A))
		{
			m_CameraPosition -= glm::normalize(glm::cross(m_Camera.GetFront(), m_Camera.GetUp())) * m_CameraTranslationSpeed * (float)ts;
		}
		if (Input::IsKeyPressed(LP_KEY_D))
		{
			m_CameraPosition += glm::normalize(glm::cross(m_Camera.GetFront(), m_Camera.GetUp())) * m_CameraTranslationSpeed * (float)ts;
		}

		m_Camera.SetPosition(m_CameraPosition);
	}

	void PerspectiveCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(LP_BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));
		dispatcher.Dispatch<MouseMovedEvent>(LP_BIND_EVENT_FN(PerspectiveCameraController::OnMouseMoved));
	}

	bool PerspectiveCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);

		return true;
	}

	bool PerspectiveCameraController::OnMouseMoved(MouseMovedEvent& e)
	{
		float xOffset = e.GetX() - m_LastX;
		float yOffset = m_LastY - e.GetY();
		m_LastX = e.GetX();
		m_LastY = e.GetY();

		float sensitivity = 0.15f;
		xOffset *= sensitivity;
		yOffset *= sensitivity;

		m_Camera.SetYaw(m_Camera.GetYaw() + xOffset);
		m_Camera.SetPitch(m_Camera.GetPitch() + yOffset);

		if (m_Camera.GetPitch() > 89.f)
		{
			m_Camera.SetPitch(89.f);
		}
		if (m_Camera.GetPitch() < -89.f)
		{
			m_Camera.SetPitch(-89.f);
		}

		glm::vec3 dir;
		dir.x = cos(glm::radians(m_Camera.GetYaw()) * cos(glm::radians(m_Camera.GetPitch())));
		dir.y = sin(glm::radians(m_Camera.GetPitch()));
		dir.z = sin(glm::radians(m_Camera.GetYaw()) * cos(glm::radians(m_Camera.GetPitch())));

		m_Camera.SetFront(glm::normalize(dir));
		m_Camera.SetRotation({ m_Camera.GetYaw(), m_Camera.GetPitch(), 0.f });
		return true;
	}
}