#pragma once

#include <Lamp/Rendering/Cameras/PerspectiveCameraController.h>
#include <Lamp/Audio/AudioEngine.h>

#include <glm/gtc/quaternion.hpp>

namespace Sandbox3D
{
	class SandboxController
	{
	public:
		SandboxController();

		//Getting
		inline Ref<Lamp::PerspectiveCameraController>& GetCameraController() { return m_CameraController; }

		void Update(Lamp::Timestep ts);
		void OnEvent(Lamp::Event& e) { m_CameraController->OnEvent(e); }

	private:
		Ref<Lamp::PerspectiveCameraController> m_CameraController;
		Lamp::Listener m_Listener;
	};
}