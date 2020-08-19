#include "Game.h"

void Game::OnStart()
{
	m_pPerspectiveCamera = std::make_shared<Lamp::PerspectiveCameraController>(60.f, 0.1f, 100.f);
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
	m_pPerspectiveCamera->Update(e.GetTimestep());

	return true;
}

void Game::OnEvent(Lamp::Event& e)
{
	m_pPerspectiveCamera->OnEvent(e);

	Lamp::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(Game::OnUpdate));
}