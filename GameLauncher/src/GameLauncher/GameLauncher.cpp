#include "GameLauncher.h"

#include <Lamp/Objects/Entity/BaseComponents/CameraComponent.h>
#include <Lamp/Rendering/RenderPass.h>

namespace GameLauncher
{
	GameLauncher::GameLauncher()
	{
		m_pGame = CreateScope<Game>();
		m_pGame->OnStart();

		for (auto& e : g_pEnv->pEntityManager->GetEntities())
		{
			auto entity = e.second;

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
		for (auto& e : g_pEnv->pEntityManager->GetEntities())
		{
			auto entity = e.second;
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
		using namespace Lamp;

		///////Shadow pass/////
		//{
		//	FramebufferSpecification shadowBuffer;
		//	shadowBuffer.Attachments =
		//	{
		//		{ FramebufferTextureFormat::DEPTH32F, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToBorder }
		//	};
		//	shadowBuffer.ClearColor = m_ClearColor;
		//	shadowBuffer.Height = 4096;
		//	shadowBuffer.Width = 4096;

		//	RenderPassSpecification shadowSpec;
		//	shadowSpec.TargetFramebuffer = CreateRef<Lamp::OpenGLFramebuffer>(shadowBuffer);
		//	shadowSpec.Camera = m_SandboxController->GetCameraController()->GetCamera();
		//	shadowSpec.IsShadowPass = true;

		//	Ref<RenderPass> shadowPass = CreateRef<RenderPass>(shadowSpec);
		//	RenderPassManager::Get()->AddPass(shadowPass);
		//}
		///////////////////////

		///////Point shadow pass/////
		//{
		//	RenderPassSpecification shadowSpec;
		//	shadowSpec.Camera = m_SandboxController->GetCameraController()->GetCamera();
		//	shadowSpec.IsPointShadowPass = true;

		//	Ref<RenderPass> shadowPass = CreateRef<RenderPass>(shadowSpec);
		//	RenderPassManager::Get()->AddPass(shadowPass);
		//}
		/////////////////////////////

		///////Main//////
		//{
		//	FramebufferSpecification mainBuffer;
		//	mainBuffer.Attachments =
		//	{
		//		{ FramebufferTextureFormat::RGBA8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge },
		//		{ FramebufferTextureFormat::RED_INTEGER, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::Repeat },
		//		{ FramebufferTextureFormat::DEPTH24STENCIL8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge }
		//	};
		//	mainBuffer.ClearColor = m_ClearColor;
		//	mainBuffer.Height = 1280;
		//	mainBuffer.Width = 720;
		//	mainBuffer.Samples = 1;

		//	std::vector<std::function<void()>> ptrs;
		//	ptrs.push_back(LP_EXTRA_RENDER(Sandbox3D::RenderGrid));
		//	ptrs.push_back(LP_EXTRA_RENDER(Sandbox3D::RenderSkybox));

		//	RenderPassSpecification passSpec;
		//	passSpec.Camera = m_SandboxController->GetCameraController()->GetCamera();
		//	passSpec.ExtraRenders = ptrs;

		//	// TODO: Fix issue with not being able to use Lamp::Framebuffer::Create(mainBuffer);
		//	passSpec.TargetFramebuffer = CreateRef<Lamp::OpenGLFramebuffer>(mainBuffer);

		//	m_SandboxBuffer = passSpec.TargetFramebuffer;

		//	Ref<RenderPass> renderPass = CreateRef<RenderPass>(passSpec);
		//	RenderPassManager::Get()->AddPass(renderPass);
		//}
		/////////////////
	}
}