#pragma once

#include <Lamp.h>

#include <Game/Game.h>

namespace GameLauncher
{
	class GameLauncher : public Lamp::Layer
	{
	public:
		GameLauncher();

		bool OnUpdate(Lamp::AppUpdateEvent& e);
		virtual void OnImGuiRender(Lamp::Timestep ts) override;
		virtual void OnEvent(Lamp::Event& e) override;

	private:
		Scope<Game> m_pGame;
	};
}