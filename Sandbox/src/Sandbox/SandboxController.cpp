#include "SandboxController.h"

#include <Lamp/Level/Level.h>
#include <Lamp/Level/LevelManager.h>

namespace Sandbox
{
	using namespace Lamp;

	SandboxController::SandboxController()
		: m_Listener(0)
	{
		m_CameraController = CreateRef<Lamp::PerspectiveCameraController>(60.f, 0.1f, 1000.f);
	}

	void SandboxController::Update(Lamp::Timestep ts)
	{
		m_CameraController->Update(ts);

		if (LevelManager::IsLevelLoaded())
		{
			auto& env = LevelManager::GetActive()->GetEnvironment();

			env.SetCameraPosition(m_CameraController->GetCamera()->GetPosition());
			env.SetCameraRotation(glm::quat(m_CameraController->GetCamera()->GetRotation()));

			Lamp::ListenerAttributes attr(m_CameraController->GetPosition(), glm::vec3(0.f), - std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->GetForwardDirection(), std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->GetUpDirection());
			Lamp::AudioEngine::Get()->SetListenerAttributes(m_Listener, attr);
		}
	}

	void SandboxController::OnEvent(Lamp::Event& e)
	{
		m_CameraController->OnEvent(e);
	
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<LevelLoadFinishedEvent>(LP_BIND_EVENT_FN(SandboxController::OnLevelLoadedFinished));
	}

	bool SandboxController::OnLevelLoadedFinished(Lamp::LevelLoadFinishedEvent& e)
	{
		m_CameraController = CreateRef<Lamp::PerspectiveCameraController>(LevelManager::GetActive()->GetEnvironment().GetCameraFOV(), 0.1f, 1000.f);
		m_CameraController->SetPosition(LevelManager::GetActive()->GetEnvironment().GetCameraPosition());
		m_CameraController->SetRotation(glm::eulerAngles(LevelManager::GetActive()->GetEnvironment().GetCameraRotation()));

		std::dynamic_pointer_cast<Lamp::PerspectiveCamera>(m_CameraController->GetCamera())->UpdateVectors();

		return false;
	}
}