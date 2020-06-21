#include "lppch.h"
#include "PerspectiveCameraController.h"
#include "Lamp/Input/Input.h"
#include "Lamp/Input/KeyCodes.h"

#include "Lamp/Core/Window.h"
#include "Lamp/Core/Application.h"

#include "Lamp/Rendering/Renderer3D.h"

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
		if (Input::IsMouseButtonPressed(1))
		{
			m_RightMouseButtonPressed = true;
			Application::Get().GetWindow().ShowCursor(false);
		}
		if (Input::IsMouseButtonReleased(1))
		{
			m_RightMouseButtonPressed = false;
			Application::Get().GetWindow().ShowCursor(true);
			m_HasControl = false;
		}

		if (m_RightMouseButtonPressed)
		{
			if (Input::IsKeyPressed(LP_KEY_W))
			{
				m_CameraPosition += m_TranslationSpeed * m_Camera.GetFront() * (float)ts;
			}
			if (Input::IsKeyPressed(LP_KEY_S))
			{
				m_CameraPosition -= m_TranslationSpeed * m_Camera.GetFront() * (float)ts;
			}
			if (Input::IsKeyPressed(LP_KEY_A))
			{
				m_CameraPosition -= m_Camera.GetRight() * m_TranslationSpeed * (float)ts;
			}
			if (Input::IsKeyPressed(LP_KEY_D))
			{
				m_CameraPosition += m_Camera.GetRight() * m_TranslationSpeed * (float)ts;
			}
		}

		m_Camera.SetPosition(m_CameraPosition);
	}

	void PerspectiveCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(LP_BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));
		dispatcher.Dispatch<MouseScrolledEvent>(LP_BIND_EVENT_FN(PerspectiveCameraController::OnMouseScrolled));
		dispatcher.Dispatch<AppRenderEvent>(LP_BIND_EVENT_FN(PerspectiveCameraController::OnRender));
	}

	void PerspectiveCameraController::UpdatePerspective(float width, float height)
	{
		m_AspectRatio = width / height;
		m_Camera.SetProjection(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
	}

	glm::vec3 PerspectiveCameraController::ScreenToWorldCoords(const glm::vec2& coords, const glm::vec2& size)
	{
		float x = (coords.x / size.x) * 2.f - 1.f;
		float y = (coords.y / size.y) * 2.f - 1.f;
		float z = 1.f;

		glm::mat4 matInv = glm::inverse(m_Camera.GetViewProjectionMatrix());
		glm::vec4 dCoords = matInv * glm::vec4(x, -y, z, 1);

		glm::vec3 dir = glm::vec3(dCoords.x, dCoords.y, dCoords.z);
		dir = glm::normalize(dir);

		return dir;
	}

	bool PerspectiveCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);

		return true;
	}

	bool PerspectiveCameraController::OnMouseMoved(const glm::vec2& e)
	{
		if (m_RightMouseButtonPressed)
		{
			if (!m_HasControl)
			{
				m_LastX = e.x;
				m_LastY = e.y;

				m_HasControl = true;
			}

			float xOffset = e.x - m_LastX;
			float yOffset = m_LastY - e.y;
			m_LastX = e.x;
			m_LastY = e.y;

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

			m_Camera.UpdateVectors();
		}
		return true;
	}

	bool PerspectiveCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		if (m_RightMouseButtonPressed)
		{
			m_TranslationSpeed += e.GetYOffset() * 0.5f;
			m_TranslationSpeed = std::min(m_TranslationSpeed, m_MaxTranslationSpeed);
			if (m_TranslationSpeed < 0)
			{
				m_TranslationSpeed = 0;
			}
		}
		return true;
	}

	bool PerspectiveCameraController::OnRender(AppRenderEvent& e)
	{
		return true;
	}
}