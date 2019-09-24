#pragma once

#include "Lamp/Core/Timestep.h"
#include "Lamp/Rendering/OrthographicCamera.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Event/MouseEvent.h"

namespace Lamp
{
	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void Update(Timestep ts);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.f;
		OrthographicCamera m_Camera;

		bool m_Rotation;

		glm::vec3 m_CameraPosition = { 0.f, 0.f, 0.f };
		float m_CameraRotation = 0.f;
		float m_CameraTranslationSpeed = 5.f, m_CameraRotationSpeed = 180.f;
	};
}