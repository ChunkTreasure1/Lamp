#include "GameLauncher.h"

namespace GameLauncher
{
	GameLauncher::GameLauncher()
	{
		m_pGame = std::make_unique<Game>();
		m_pGame->OnStart();
	}

	bool GameLauncher::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		Lamp::Renderer::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
		Lamp::Renderer::Clear();

		Lamp::Renderer3D::Begin(m_pGame->GetCamera()->GetCamera());

		Lamp::AppRenderEvent renderEvent;
		Lamp::ObjectLayerManager::Get()->OnEvent(renderEvent);
		m_pGame->OnEvent(renderEvent);

		Lamp::Renderer3D::DrawSkybox();
		Lamp::Renderer3D::End();

		return true;
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