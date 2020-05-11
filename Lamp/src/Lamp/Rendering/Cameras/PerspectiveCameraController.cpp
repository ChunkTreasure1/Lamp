#include "lppch.h"
#include "PerspectiveCameraController.h"
#include "Lamp/Input/Input.h"
#include "Lamp/Input/KeyCodes.h"

#include "Lamp/Core/Window.h"
#include "Lamp/Core/Application.h"
#include "Lamp/Physics/Physics.h"

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
			m_LastHadControl = true;
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
				m_CameraPosition += m_CameraTranslationSpeed * m_Camera.GetFront() * (float)ts;
			}
			if (Input::IsKeyPressed(LP_KEY_S))
			{
				m_CameraPosition -= m_CameraTranslationSpeed * m_Camera.GetFront() * (float)ts;
			}
			if (Input::IsKeyPressed(LP_KEY_A))
			{
				m_CameraPosition -= m_Camera.GetRight() * m_CameraTranslationSpeed * (float)ts;
			}
			if (Input::IsKeyPressed(LP_KEY_D))
			{
				m_CameraPosition += m_Camera.GetRight() * m_CameraTranslationSpeed * (float)ts;
			}
		}

		m_Camera.SetPosition(m_CameraPosition);
	}

	void PerspectiveCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(LP_BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));
		dispatcher.Dispatch<MouseMovedEvent>(LP_BIND_EVENT_FN(PerspectiveCameraController::OnMouseMoved));
		dispatcher.Dispatch<MouseScrolledEvent>(LP_BIND_EVENT_FN(PerspectiveCameraController::OnMouseScrolled));
	}

	void PerspectiveCameraController::UpdatePerspective(float width, float height)
	{
		m_AspectRatio = width / height;
		m_Camera.SetProjection(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
		glViewport(0, 0, width, height);
	}

	glm::vec3 PerspectiveCameraController::ScreenToWorldCoords(const glm::vec2& coords, const glm::vec2& size)
	{
		glm::vec3 pos0;
		if (!Unproject(glm::vec3(coords.x, coords.y, -1), pos0, size))
		{
			return glm::vec3(0, 0, 0);
		}

		glm::vec3 pos1;
		if (!Unproject(glm::vec3(coords.x, coords.y, 1), pos1, size))
		{
			return glm::vec3(0.f, 0.f, 0.f);
		}

		glm::vec3 v = (pos1 - pos0);
		v = glm::normalize(v);

		if (!_finite(v.x) || !_finite(v.y) || !_finite(v.z))
		{
			return glm::vec3(0, 0, 0);
		}

		glm::vec3 colp(0, 0, 0);

		glm::vec3 vPos(pos0.x, pos0.y, pos0.z);
		glm::vec3 vDir(v.x, v.y, v.z);

		Ray ray;
		ray.origin = m_Camera.GetPosition();
		ray.direction = vDir;

		SpherePhysicsObject obj(3.f);
		obj.SetPosition({ 0.f, 0.f, 0.f });

		bool result = Physics::IntersectRaySphere(ray, obj);
	}

	bool PerspectiveCameraController::Unproject(const glm::vec3& viewPos, glm::vec3& result, const glm::vec2& size)
	{
		glm::vec4 vIn;
		vIn.x = (viewPos.x / size.x) * 2.f - 1;
		vIn.y = (viewPos.y / size.y) * 2.f - 1;
		vIn.z = viewPos.z;
		vIn.w = 1.f;

		glm::mat4 mInv = glm::inverse(m_Camera.GetViewProjectionMatrix());
		glm::vec4 vOut =  mInv * vIn;
		if (vOut.w == 0.f)
		{
			return false;
		}

		result = glm::vec3(vOut.x / vOut.w, vOut.y / vOut.w, vOut.z / vOut.w);

		return true;
	}

	bool PerspectiveCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);

		return true;
	}

	bool PerspectiveCameraController::OnMouseMoved(MouseMovedEvent& e)
	{
		if (m_RightMouseButtonPressed)
		{
			if (!m_HasControl)
			{
				m_LastX = e.GetX();
				m_LastY = e.GetY();

				m_HasControl = true;
			}

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

			m_Camera.UpdateVectors();
		}
		return true;
	}
	bool PerspectiveCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		if (m_RightMouseButtonPressed)
		{
			m_CameraTranslationSpeed += e.GetYOffset() * 0.5f;
			m_CameraTranslationSpeed = std::min(m_CameraTranslationSpeed, 10.f);
			if (m_CameraTranslationSpeed < 0)
			{
				m_CameraTranslationSpeed = 0;
			}
		}
		return true;
	}
}