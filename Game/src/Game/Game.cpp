#include "Game.h"

#include <Lamp/Meshes/GeometrySystem.h>
#include <Lamp//Objects/Entity/BaseComponents/LightComponent.h>
#include <Lamp//Objects/Entity/BaseComponents/MeshComponent.h>

void Game::OnStart()
{
	auto tempLevel = Lamp::LevelSystem::LoadLevel("assets/levels/Level.level");

	{
		Lamp::Entity* ent = Lamp::EntityManager::Get()->Create();
		ent->SetPosition({ 10, 0, 0 });

		auto comp = ent->GetOrCreateComponent<Lamp::MeshComponent>();
		comp->SetModel(Lamp::GeometrySystem::LoadFromFile("assets/models/test.lgf"));
	}

	{
		Lamp::Entity* ent2 = Lamp::EntityManager::Get()->Create();
		ent2->SetPosition({ 0.f, 7.f, -2.f });
		auto comp = ent2->GetOrCreateComponent<Lamp::MeshComponent>();
		comp->SetModel(Lamp::GeometrySystem::LoadFromFile("assets/models/lightModel.lgf"));

		auto light = ent2->GetOrCreateComponent<Lamp::LightComponent>();
		light->SetAmbient({ 0.2f, 0.2f, 0.2f });
		light->SetDiffuse({ 0.4f, 0.4f, 0.4f });
		light->SetSpecular({ 1.f, 1.f, 1.f });
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