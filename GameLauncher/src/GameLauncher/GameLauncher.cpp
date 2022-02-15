#include "GameLauncher.h"

#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>

namespace GameLauncher
{
	GameLauncher::GameLauncher()
	{
		m_pGame = CreateScope<Game>();
		m_pGame->OnStart();
	}

	bool GameLauncher::OnUpdate(Lamp::AppUpdateEvent& e)
	{

		return false;
	}

	void GameLauncher::OnImGuiRender(Lamp::Timestep ts)
	{
	}

	void GameLauncher::OnEvent(Lamp::Event& e)
	{
		m_pGame->OnEvent(e);

		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(GameLauncher::OnUpdate));
	}
}