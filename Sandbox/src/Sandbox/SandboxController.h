#pragma once

#include <Lamp/Event/EntityEvent.h>
#include <Lamp/Rendering/Cameras/PerspectiveCameraController.h>
#include <Lamp/Audio/AudioEngine.h>

#include <glm/gtc/quaternion.hpp>

namespace Sandbox
{
	class SandboxController
	{
	public:
		SandboxController();

		//Getting
		inline Ref<Lamp::PerspectiveCameraController>& GetCameraController() { return m_CameraController; }

		void Update(Lamp::Timestep ts);
		void OnEvent(Lamp::Event& e);

	private:
		bool OnLevelLoadedFinished(Lamp::LevelLoadFinishedEvent& e);

		Ref<Lamp::PerspectiveCameraController> m_CameraController;
		Lamp::Listener m_Listener;
	};
}