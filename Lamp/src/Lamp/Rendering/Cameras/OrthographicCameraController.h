#pragma once

#include "Lamp/Core/Time/Timestep.h"
#include "Lamp/Rendering/Cameras/OrthographicCamera.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Event/MouseEvent.h"

namespace Lamp
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);
		~OrthographicCameraController();

		void Update(Timestep ts);
		void OnEvent(Event& e);

		//Getting
		OrthographicCamera GetCamera() { return m_Camera; }
		const OrthographicCamera GetCamera() const { return m_Camera; }

		//Setting
		void SetAspectRatio(float ratio) { m_AspectRatio = ratio; }
		void SetHasControl(bool state) { m_HasControl = state; }
		void SetPosition(const glm::vec3& somePos) { m_CameraPosition = somePos; }

		glm::vec2 ScreenToWorldCoords(const glm::vec2& coords, const glm::vec2& size);
	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.5f;
		OrthographicCamera m_Camera;

		bool m_Rotation;
		bool m_HasControl;

		glm::vec3 m_CameraPosition = { 0.f, 0.f, 0.f };
		float m_CameraRotation = 0.f;
		float m_CameraTranslationSpeed = 5.f, m_CameraRotationSpeed = 180.f;
	};
}