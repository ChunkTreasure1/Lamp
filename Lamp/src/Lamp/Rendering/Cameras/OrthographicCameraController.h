#pragma once

#include "Lamp/Core/Time/Timestep.h"
#include "Lamp/Rendering/Cameras/OrthographicCamera.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Event/MouseEvent.h"

#include "CameraControllerBase.h"

namespace Lamp
{
	class OrthographicCameraController : public CameraControllerBase
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);
		virtual ~OrthographicCameraController() override;

		virtual void Update(Timestep ts) override;
		virtual void OnEvent(Event& e) override;
		virtual void UpdateProjection(uint32_t width, uint32_t height) override;
		virtual glm::vec3 ScreenToWorldCoords(const glm::vec2& coords, const glm::vec2& size) override;

	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.5f;

		bool m_Rotation;
		bool m_ControlsEnabled = false;

		glm::vec3 m_CameraPosition = { 0.f, 0.f, 0.f };
		float m_CameraRotation = 0.f;
		float m_CameraTranslationSpeed = 5.f, m_CameraRotationSpeed = 180.f;
	};
}