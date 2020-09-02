#include "Game.h"

#include <Lamp/Meshes/GeometrySystem.h>
#include <Lamp/Objects/Entity/BaseComponents/LightComponent.h>
#include <Lamp/Objects/Entity/BaseComponents/MeshComponent.h>

#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>

void Game::OnStart()
{
	auto tempLevel = Lamp::LevelSystem::LoadLevel("assets/levels/Level.level");

	Lamp::BrushManager::Get()->Create("assets/models/test.lgf");

	{
		Lamp::Entity* ent = Lamp::EntityManager::Get()->Create();
		ent->SetPosition({ 0, 0, 0 });

		auto comp = ent->GetOrCreateComponent<Lamp::LightComponent>();
		auto mesh = ent->GetOrCreateComponent<Lamp::MeshComponent>();
		mesh->SetModel(Lamp::GeometrySystem::LoadFromFile("assets/models/lightModel.lgf"));
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