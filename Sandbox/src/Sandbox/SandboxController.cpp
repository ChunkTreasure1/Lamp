#include "SandboxController.h"

#include "Lamp/Level/Level.h"

namespace Sandbox
{
	SandboxController::SandboxController()
		: m_Listener(0)
	{
		m_CameraController = CreateRef<Lamp::PerspectiveCameraController>(g_pEnv->pLevel->GetEnvironment().CameraFOV, 0.1f, 1000.f);
		m_CameraController->SetPosition(g_pEnv->pLevel->GetEnvironment().CameraPosition);
		m_CameraController->SetRotation(glm::eulerAngles(g_pEnv->pLevel->GetEnvironment().CameraRotation));

		std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->UpdateVectors();
	}

	void SandboxController::Update(Lamp::Timestep ts)
	{
		m_CameraController->Update(ts);
		g_pEnv->pLevel->GetEnvironment().CameraPosition = m_CameraController->GetCamera()->GetPosition();
		g_pEnv->pLevel->GetEnvironment().CameraRotation = glm::quat(m_CameraController->GetCamera()->GetRotation());
		g_pEnv->pLevel->GetEnvironment().CameraFOV = m_CameraController->GetFOV();

		Lamp::ListenerAttributes attr(m_CameraController->GetPosition(), glm::vec3(0.f), -std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->GetFront(), std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->GetUp());
		Lamp::AudioEngine::Get()->SetListenerAttributes(m_Listener, attr);
	}
}