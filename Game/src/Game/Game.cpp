#include "Game.h"

#include <Lamp/Meshes/GeometrySystem.h>
#include <Lamp/Objects/Entity/BaseComponents/LightComponent.h>
#include <Lamp/Objects/Entity/BaseComponents/MeshComponent.h>

#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>
#include <Lamp/Audio/AudioEngine.h>
#include <Lamp/Objects/Entity/BaseComponents/AudioComponent.h>
#include <Lamp/Objects/Entity/BaseComponents/AudioListenerComponent.h>

void Game::OnStart()
{
	auto tempLevel = Lamp::LevelSystem::LoadLevel("assets/levels/Level.level");
	{
		Lamp::Entity* ent = Lamp::Entity::Create();
		ent->SetPosition({ 0, 0, 0 });

		auto comp = ent->GetOrCreateComponent<Lamp::LightComponent>();
		auto mesh = ent->GetOrCreateComponent<Lamp::MeshComponent>();
		mesh->SetModel(Lamp::GeometrySystem::LoadFromFile("assets/models/lightModel.lgf"));
		auto audio = ent->GetOrCreateComponent<Lamp::AudioComponent>();

		//audio->SetEvent("test");
		//audio->Play();
	}

	{
		Lamp::Entity* ent = Lamp::Entity::Create();
		ent->GetOrCreateComponent<Lamp::AudioListenerComponent>();
		ent->SetPosition(glm::vec3(10.f, 0.f, 0.f));
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