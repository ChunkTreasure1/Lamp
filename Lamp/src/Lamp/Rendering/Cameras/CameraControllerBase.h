#pragma once

#include "Lamp/Core/Time/Timestep.h"
#include "Lamp/Event/Event.h"

#include <glm/glm.hpp>
#include "CameraBase.h"

namespace Lamp
{
	class CameraControllerBase
	{
	public:
		CameraControllerBase()
			: m_AspectRatio(16.f/9.f)
		{}
		virtual ~CameraControllerBase() {}

		virtual void Update(Timestep ts) = 0;
		virtual void OnEvent(Event& e) = 0;
		virtual void UpdateProjection(uint32_t width, uint32_t height) = 0;
		virtual glm::vec3 ScreenToWorldCoords(const glm::vec2& coords, const glm::vec2& size) = 0;

		//Getting
		Ref<CameraBase>& GetCamera() { return m_Camera; }
		const Ref<CameraBase>& GetCamera() const { return m_Camera; }
		inline const glm::vec3& GetPosition() { return m_Position; }

		//Setting
		inline void SetAspectRatio(float ratio) { m_AspectRatio = ratio; }
		inline void SetPosition(const glm::vec3& pos) { m_Position = pos; m_Camera->SetPosition(pos); }
		inline void SetRotation(const glm::vec3& rot) { m_Rotation = rot; m_Camera->SetRotation(rot); }
		inline void SetControlsEnabled(bool state) { m_ControlsEnabled = state; }

	protected:
		Ref<CameraBase> m_Camera;

		glm::vec3 m_Position = { 0.f, 0.f, 0.f };
		glm::vec3 m_Rotation = { 0.f, 0.f, 0.f };

		float m_AspectRatio;
		bool m_ControlsEnabled = false;
	};
}