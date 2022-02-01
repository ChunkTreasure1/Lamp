#include "SandboxController.h"

#include "Lamp/Level/Level.h"

namespace Sandbox
{
	SandboxController::SandboxController()
		: m_Listener(0)
	{
		m_CameraController = CreateRef<Lamp::PerspectiveCameraController>(g_pEnv->pLevel->GetEnvironment().GetCameraFOV(), 0.1f, 1000.f);
		m_CameraController->SetPosition(g_pEnv->pLevel->GetEnvironment().GetCameraPosition());
		m_CameraController->SetRotation(glm::eulerAngles(g_pEnv->pLevel->GetEnvironment().GetCameraRotation()));

		std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->UpdateVectors();
	}

	void SandboxController::Update(Lamp::Timestep ts)
	{
		m_CameraController->Update(ts);

		if (g_pEnv->pLevel)
		{
			auto& env = g_pEnv->pLevel->GetEnvironment();

			env.SetCameraPosition(m_CameraController->GetCamera()->GetPosition());
			env.SetCameraRotation(glm::quat(m_CameraController->GetCamera()->GetRotation()));

			Lamp::ListenerAttributes attr(m_CameraController->GetPosition(), glm::vec3(0.f), -std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->GetForwardDirection(), std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->GetUpDirection());
			Lamp::AudioEngine::Get()->SetListenerAttributes(m_Listener, attr);
		}
	}
}