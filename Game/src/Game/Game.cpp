#include "Game.h"

#include <Lamp.h>

#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>
#include "ControllerComponent.h"

void Game::OnStart()
{
	for (auto& ent : g_pEnv->pLevel->GetEntities())
	{
		if (ent.second->HasComponent<Lamp::CameraComponent>())
		{
			ent.second->GetOrCreateComponent<ControllerComponent>();
		}
	}
}

void Game::OnStop()
{
}

bool Game::OnUpdate(Lamp::AppUpdateEvent& e)
{
	return false;
}

void Game::OnEvent(Lamp::Event& e)
{
	Lamp::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(Game::OnUpdate));
}