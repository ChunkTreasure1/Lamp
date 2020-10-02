#pragma once

#include <Lamp/Rendering/Cameras/PerspectiveCameraController.h>
#include <Lamp/Audio/AudioEngine.h>

namespace Sandbox3D
{
	class SandboxController
	{
	public:
		SandboxController()
			: m_Listener(0)
		{
			m_CameraController = CreateRef<Lamp::PerspectiveCameraController>(Lamp::LevelSystem::GetEnvironment().CameraFOV, 0.1f, 100.f);
			m_CameraController->SetPosition(Lamp::LevelSystem::GetEnvironment().CameraPosition);
			m_CameraController->SetRotation(Lamp::LevelSystem::GetEnvironment().CameraRotation);
		}

		//Getting
		inline Ref<Lamp::PerspectiveCameraController>& GetCameraController() { return m_CameraController; }

		void Update(Lamp::Timestep ts)
		{
			m_CameraController->Update(ts);
			Lamp::LevelSystem::GetEnvironment().CameraPosition = m_CameraController->GetCamera()->GetPosition();
			Lamp::LevelSystem::GetEnvironment().CameraRotation = m_CameraController->GetCamera()->GetRotation();
			Lamp::LevelSystem::GetEnvironment().CameraFOV = m_CameraController->GetFOV();

			Lamp::ListenerAttributes attr(m_CameraController->GetPosition(), glm::vec3(0.f), -std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->GetFront(), std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->GetUp());
			Lamp::AudioEngine::Get()->SetListenerAttributes(m_Listener, attr);
		}

		void OnEvent(Lamp::Event& e) { m_CameraController->OnEvent(e); }

	private:
		Ref<Lamp::PerspectiveCameraController> m_CameraController;
		Lamp::Listener m_Listener;
	};
}