#include "GameLauncher.h"

#include <Lamp/Rendering/RenderCommand.h>
#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>

namespace GameLauncher
{
	GameLauncher::GameLauncher()
	{
		m_pGame = CreateScope<Game>();
		m_pGame->OnStart();
	}

	bool GameLauncher::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		Lamp::RenderCommand::SetClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 1.f));
		Lamp::RenderCommand::Clear();

		//Improve
		for (auto& entity : Lamp::EntityManager::Get()->GetEntities())
		{
			if (auto& comp = entity->GetComponent<Lamp::CameraComponent>())
			{
				if (comp->GetIsMain())
				{
					m_Camera = comp->GetCamera();
				}
			}
		}

		Lamp::Renderer3D::Begin(m_Camera);
		Lamp::Renderer3D::DrawLine(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.1f, 0.1f, 0.f), 0.1f);


		Lamp::AppRenderEvent renderEvent;
		Lamp::ObjectLayerManager::Get()->OnEvent(renderEvent);
		OnEvent(e);

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