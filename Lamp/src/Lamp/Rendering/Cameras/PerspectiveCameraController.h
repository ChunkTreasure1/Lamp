#pragma once

#include "PerspectiveCamera.h"
#include "Lamp/Core/Timestep.h"
#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Event/MouseEvent.h"

namespace Lamp
{
	class PerspectiveCameraController
	{
	public:
		PerspectiveCameraController(float fov, float nearPlane, float farPlane);
		~PerspectiveCameraController();

		void Update(Timestep ts);
		void OnEvent(Event& e);

		//Getting
		PerspectiveCamera GetCamera() { return m_Camera; }
		const PerspectiveCamera GetCamera() const { return m_Camera; }

		inline void SetPosition(const glm::vec3& somePos) { m_CameraPosition = somePos; }
		void UpdatePerspective(float width, float height);
		inline void SetHasControl(bool state) { if (m_HasControl && !state) { m_LastHadControl = true; } m_HasControl = state; }

		glm::vec3 ScreenToWorldCoords(const glm::vec2& coords, const glm::vec2& size);
		bool Unproject(const glm::vec3& viewPos, glm::vec3& result, const glm::vec2& size, glm::vec2 topLeft = glm::vec2(0, 0), glm::vec2 widthHeight = glm::vec2(0, 0));

	private:
		bool OnWindowResized(WindowResizeEvent& e);
		bool OnMouseMoved(MouseMovedEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);

	private:
		PerspectiveCamera m_Camera;
		glm::vec3 m_CameraPosition = { 0.f, 0.f, 0.f };

		float m_CameraTranslationSpeed = 5.f;
		float m_AspectRatio = 0.f;
		float m_NearPlane = 0.1f;
		float m_FarPlane = 100.f;
		float m_FOV = 45.f;

		float m_LastX = 1280 / 2;
		float m_LastY = 720 / 2;
		bool m_HasControl = false;
		bool m_LastHadControl = false;
		bool m_RightMouseButtonPressed = false;
	};
}