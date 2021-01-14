#include "GameLauncher.h"

#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>
#include <Lamp/Rendering/RenderPass.h>

namespace GameLauncher
{
	GameLauncher::GameLauncher()
	{
		m_pGame = CreateScope<Game>();
		m_pGame->OnStart();

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
		CreateRenderPasses();
	}

	bool GameLauncher::OnUpdate(Lamp::AppUpdateEvent& e)
	{
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

		Lamp::RenderPassManager::Get()->RenderPasses();

		return false;
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

	void GameLauncher::CreateRenderPasses()
	{
		Lamp::RenderPassInfo passInfo;
		passInfo.Camera = m_Camera;
		passInfo.IsShadowPass = true;
		passInfo.DirLight = g_pEnv->DirLight;
		passInfo.ClearColor = glm::vec4(1.f, 1.f, 1.f, 1.f);

		Ref<Lamp::RenderPass> shadowPass = CreateRef<Lamp::RenderPass>(Lamp::Renderer3D::GetShadowBuffer(), passInfo);
		Lamp::RenderPassManager::Get()->AddPass(shadowPass);

		passInfo.IsShadowPass = false;
		Ref<Lamp::RenderPass> renderPass = CreateRef<Lamp::RenderPass>(Lamp::Renderer3D::GetFrameBuffer(), passInfo);
		Lamp::RenderPassManager::Get()->AddPass(renderPass);
	}
}