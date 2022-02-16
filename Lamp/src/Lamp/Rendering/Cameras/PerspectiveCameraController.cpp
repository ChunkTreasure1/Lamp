#include "lppch.h"
#include "PerspectiveCameraController.h"
#include "Lamp/Input/Input.h"
#include "Lamp/Input/KeyCodes.h"
#include "Lamp/Input/MouseButtonCodes.h"

#include "Lamp/Core/Window.h"
#include "Lamp/Core/Application.h"

#include "PerspectiveCamera.h"

namespace Lamp
{
	PerspectiveCameraController::PerspectiveCameraController(float fov, float nearPlane, float farPlane)
		: m_fov(fov), m_nearPlane(nearPlane), m_farPlane(farPlane)
	{
		m_Camera = CreateRef<PerspectiveCamera>(fov, nearPlane, farPlane);
	}

	PerspectiveCameraController::~PerspectiveCameraController()
	{
	}

	void PerspectiveCameraController::Update(Timestep ts)
	{
		if (!m_isActive)
		{
			return;
		}

		auto perspectiveCamera = std::reinterpret_pointer_cast<PerspectiveCamera>(m_Camera);

		const glm::vec2& mousePos{ Input::GetMouseX(), Input::GetMouseY() };
		const glm::vec2 deltaPos = (mousePos - m_lastMousePosition) * 0.01f;

		if (m_lastRightUp)
		{
			m_lastMousePosition = mousePos;
			
		}

		if (Input::IsMouseButtonPressed(LP_MOUSE_BUTTON_RIGHT) && !Input::IsMouseButtonPressed(LP_MOUSE_BUTTON_MIDDLE))
		{
			m_lastRightUp = false;

			float xOffset = mousePos.x - m_lastMousePosition.x;
			float yOffset = m_lastMousePosition.y - mousePos.y;

			float sensitivity = 0.15f;
			xOffset *= sensitivity;
			yOffset *= sensitivity;

			auto perspectiveCamera = std::reinterpret_pointer_cast<PerspectiveCamera>(m_Camera);

			perspectiveCamera->SetYaw(perspectiveCamera->GetYaw() + xOffset);
			perspectiveCamera->SetPitch(perspectiveCamera->GetPitch() - yOffset);

			if (perspectiveCamera->GetPitch() > 89.f)
			{
				perspectiveCamera->SetPitch(89.f);
			}
			if (perspectiveCamera->GetPitch() < -89.f)
			{
				perspectiveCamera->SetPitch(-89.f);
			}

			perspectiveCamera->UpdateVectors();

			//Movement
			if (Input::IsKeyPressed(LP_KEY_W))
			{
				m_position += m_translationSpeed * perspectiveCamera->GetForwardDirection() * (float)ts;
			}
			if (Input::IsKeyPressed(LP_KEY_S))
			{
				m_position -= m_translationSpeed * perspectiveCamera->GetForwardDirection() * (float)ts;
			}
			if (Input::IsKeyPressed(LP_KEY_A))
			{
				m_position -= m_translationSpeed * perspectiveCamera->GetRightDirection() * (float)ts;
			}
			if (Input::IsKeyPressed(LP_KEY_D))
			{
				m_position += m_translationSpeed * perspectiveCamera->GetRightDirection() * (float)ts;
			}
		}

		if (Input::IsMouseButtonReleased(LP_MOUSE_BUTTON_RIGHT))
		{
			m_lastRightUp = true;
		}

		if (Input::IsMouseButtonPressed(LP_MOUSE_BUTTON_MIDDLE) && !Input::IsKeyPressed(LP_KEY_LEFT_ALT))
		{
			m_position += -perspectiveCamera->GetRightDirection() * deltaPos.x;
			m_position += perspectiveCamera->GetUpDirection() * deltaPos.y;
		}

		//g_pEnv->DirLight.UpdateProjection(m_Position);

		m_lastMousePosition = mousePos;
		m_Camera->SetPosition(m_position);

		m_currentDeltaTime = ts;
	}

	void PerspectiveCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(LP_BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));
		dispatcher.Dispatch<MouseScrolledEvent>(LP_BIND_EVENT_FN(PerspectiveCameraController::OnMouseScrolled));
	}

	void PerspectiveCameraController::UpdateProjection(uint32_t width, uint32_t height)
	{
		m_aspectRatio = (float)width / (float)height;
		m_Camera->SetProjection(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
	}

	glm::vec3 PerspectiveCameraController::ScreenToWorldCoords(const glm::vec2& coords, const glm::vec2& size)
	{
		float x = (coords.x / size.x) * 2.f - 1.f;
		float y = (coords.y / size.y) * 2.f - 1.f;
		float z = 1.f;

		glm::mat4 matInv = glm::inverse(m_Camera->GetViewProjectionMatrix());
		glm::vec4 dCoords = matInv * glm::vec4(x, -y, z, 1);

		glm::vec3 dir = glm::vec3(dCoords.x, dCoords.y, dCoords.z);
		dir = glm::normalize(dir);

		return dir;
	}

	bool PerspectiveCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_aspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera->SetProjection(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);

		return true;
	}

	bool PerspectiveCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		if (Input::IsMouseButtonPressed(LP_MOUSE_BUTTON_RIGHT) && m_isActive)
		{
			m_translationSpeed += e.GetYOffset() * 0.5f;
			m_translationSpeed = std::min(m_translationSpeed, m_maxtranslationSpeed);
			if (m_translationSpeed < 0)
			{
				m_translationSpeed = 0;
			}
		}
		else if (m_isActive)
		{
			Ref<PerspectiveCamera> camera = std::reinterpret_pointer_cast<PerspectiveCamera>(m_Camera);

			m_position += e.GetYOffset() * camera->GetForwardDirection() * m_currentDeltaTime * m_scrollTranslationSpeed;
		}

		return false;
	}
}