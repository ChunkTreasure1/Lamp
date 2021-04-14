#include "Game.h"

#include <Lamp/Objects/Entity/BaseComponents/LightComponent.h>
#include <Lamp/Objects/Entity/BaseComponents/MeshComponent.h>

#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>
#include <Lamp/Audio/AudioEngine.h>
#include <Lamp/Objects/Entity/BaseComponents/AudioListenerComponent.h>
#include <Lamp/Meshes/Materials/MaterialLibrary.h>

#include <Lamp.h>
#include <Lamp/AssetSystem/AssetManager.h>

Game::~Game()
{
	delete m_pLevel;
}

void Game::OnStart()
{
	g_pEnv->pAssetManager->LoadLevel("assets/levels/Level.level", m_pLevel);
	Lamp::Entity* pEnt = Lamp::Entity::Create();
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