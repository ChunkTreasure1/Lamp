#pragma once

#include <Lamp/Rendering/Cameras/PerspectiveCameraController.h>
#include <Lamp/Audio/AudioEngine.h>

#include <glm/gtc/quaternion.hpp>

namespace Sandbox3D
{
	class SandboxController
	{
	public:
		SandboxController()
			: m_Listener(0)
		{
			m_CameraController = CreateRef<Lamp::PerspectiveCameraController>(g_pEnv->pLevel->GetEnvironment().CameraFOV, 0.1f, 100.f);
			m_CameraController->SetPosition(g_pEnv->pLevel->GetEnvironment().CameraPosition);
			m_CameraController->SetRotation(glm::eulerAngles(g_pEnv->pLevel->GetEnvironment().CameraRotation));
		}

		//Getting
		inline Ref<Lamp::PerspectiveCameraController>& GetCameraController() { return m_CameraController; }

		void Update(Lamp::Timestep ts)
		{
			m_CameraController->Update(ts);
			g_pEnv->pLevel->GetEnvironment().CameraPosition = m_CameraController->GetCamera()->GetPosition();
			g_pEnv->pLevel->GetEnvironment().CameraRotation = glm::quat(m_CameraController->GetCamera()->GetRotation());
			g_pEnv->pLevel->GetEnvironment().CameraFOV = m_CameraController->GetFOV();

			Lamp::ListenerAttributes attr(m_CameraController->GetPosition(), glm::vec3(0.f), -std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->GetFront(), std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->GetUp());
			Lamp::AudioEngine::Get()->SetListenerAttributes(m_Listener, attr);
		}

		void OnEvent(Lamp::Event& e) { m_CameraController->OnEvent(e); }

	private:
		Ref<Lamp::PerspectiveCameraController> m_CameraController;
		Lamp::Listener m_Listener;
	};
}