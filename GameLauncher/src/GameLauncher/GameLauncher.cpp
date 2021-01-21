#include "GameLauncher.h"

#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>
#include <Lamp/Rendering/RenderPass.h>

namespace GameLauncher
{
	using namespace Lamp;

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

	void GameLauncher::CreateRenderPasses()
	{
		/////Shadow pass/////
		{
			FramebufferSpecification shadowBuffer;
			shadowBuffer.Attachments =
			{
				{ FramebufferTextureFormat::DEPTH32F, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToBorder }
			};
			shadowBuffer.ClearColor = { 0.1f, 0.1f, 0.1f, 1.f };
			shadowBuffer.Height = 4096;
			shadowBuffer.Width = 4096;

			RenderPassSpecification shadowSpec;
			shadowSpec.TargetFramebuffer = Lamp::Framebuffer::Create(shadowBuffer);
			shadowSpec.Camera = m_Camera;
			shadowSpec.IsShadowPass = true;

			Ref<RenderPass> shadowPass = CreateRef<RenderPass>(shadowSpec);
			RenderPassManager::Get()->AddPass(shadowPass);
		}
		/////////////////////

		/////Main//////
		{
			FramebufferSpecification mainBuffer;
			mainBuffer.Attachments =
			{
				{ FramebufferTextureFormat::RGBA8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge },
				{ FramebufferTextureFormat::DEPTH24STENCIL8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge }
			};
			mainBuffer.ClearColor = { 0.1f, 0.1f, 0.1f, 1.f };
			mainBuffer.Height = 1280;
			mainBuffer.Width = 720;

			RenderPassSpecification passSpec;
			passSpec.Camera = m_Camera;
			passSpec.TargetFramebuffer = Lamp::Framebuffer::Create(mainBuffer);

			Ref<RenderPass> renderPass = CreateRef<RenderPass>(passSpec);
			RenderPassManager::Get()->AddPass(renderPass);
		}
		///////////////
	}
}