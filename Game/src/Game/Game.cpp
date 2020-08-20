#include "Game.h"

#include <Lamp/Meshes/GeometrySystem.h>
#include <Lamp/Objects/Entity/BaseComponents/LightComponent.h>
#include <Lamp/Objects/Entity/BaseComponents/MeshComponent.h>

#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>

void Game::OnStart()
{
	auto tempLevel = Lamp::LevelSystem::LoadLevel("assets/levels/Level.level");

	{
		Lamp::Entity* ent = Lamp::EntityManager::Get()->Create();
		ent->SetPosition({ 10, 0, 0 });

		auto comp = ent->GetOrCreateComponent<Lamp::CameraComponent>();
		comp->SetIsMain(true);
	}
}

bool Game::OnUpdate(Lamp::AppUpdateEvent& e)
{
	return true;
}

void Game::OnEvent(Lamp::Event& e)
{
	Lamp::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(Game::OnUpdate));
}