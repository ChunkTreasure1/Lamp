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
		virtual ~PerspectiveCameraController() override;

		virtual void Update(Timestep ts) override;
		virtual void OnEvent(Event& e) override;
		virtual void UpdateProjection(uint32_t width, uint32_t height) override;
		virtual glm::vec3 ScreenToWorldCoords(const glm::vec2& coords, const glm::vec2& size) override;

		bool OnMouseMoved(MouseMovedEvent& e);
		inline float GetFOV() { return m_FOV; }

		inline void SetHasControl(bool state) { m_HasControl = state; }

	private:
		bool OnWindowResized(WindowResizeEvent& e);
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnRender(AppRenderEvent& e);

	private:
		std::vector<std::array<glm::vec3, 2>> m_LinePositions;

		float m_TranslationSpeed = 5.f;
		float m_NearPlane = 0.1f;
		float m_FarPlane = 100.f;
		float m_FOV = 45.f;
		float m_MaxTranslationSpeed = 40.f;

		float m_LastX = 1280 / 2;
		float m_LastY = 720 / 2;

		bool m_HasControl = false;
	};
}