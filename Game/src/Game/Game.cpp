#include "Game.h"

#include <Lamp.h>

#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>
#include "ControllerComponent.h"

void Game::OnStart()
{
	Lamp::Entity* pEnt = Lamp::Entity::Create();
	pEnt->GetOrCreateComponent<Lamp::CameraComponent>();
	pEnt->GetOrCreateComponent<ControllerComponent>();
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