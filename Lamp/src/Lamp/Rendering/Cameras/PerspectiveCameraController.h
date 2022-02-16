#pragma once

#include "PerspectiveCamera.h"
#include "Lamp/Core/Time/Timestep.h"
#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Event/MouseEvent.h"

#include "CameraControllerBase.h"

namespace Lamp
{
	class PerspectiveCameraController : public CameraControllerBase
	{
	public:
		PerspectiveCameraController(float fov, float nearPlane, float farPlane);
		~PerspectiveCameraController() override;

		void Update(Timestep ts) override;
		void OnEvent(Event& e) override;
		void UpdateProjection(uint32_t width, uint32_t height) override;
		glm::vec3 ScreenToWorldCoords(const glm::vec2& coords, const glm::vec2& size) override;

		inline float GetFOV() { return m_fov; }

		inline void SetActive(bool state) { m_isActive = state; }
		inline void SetHasControl(bool state) { m_hasControl = state; }

	private:
		bool OnWindowResized(WindowResizeEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);

	private:
		glm::vec2 m_lastMousePosition{0.f};

		float m_translationSpeed = 5.f;
		float m_scrollTranslationSpeed = 30.f;

		float m_fov = 45.f;
		float m_nearPlane = 0.1f;
		float m_farPlane = 100.f;

		float m_currentDeltaTime = 0.f;

		float m_maxtranslationSpeed = 40.f;

		bool m_hasControl = false;
		bool m_lastRightUp = true;
		bool m_isActive = false;
	};
}