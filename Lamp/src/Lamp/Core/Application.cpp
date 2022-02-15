#include "lppch.h"
#include "Application.h"

#include "imgui.h"

#include "Lamp/Audio/AudioEngine.h"
#include "Lamp/Physics/Physics.h"
#include "Lamp/AssetSystem/AssetManager.h"
#include "Lamp/Objects/Entity/ComponentInclude.h"
#include "Lamp/ImGui/ImGuiLayer.h"
#include "Lamp/Core/CoreLogger.h"

#include "Lamp/Rendering/Swapchain.h"
#include "Lamp/Rendering/RenderCommand.h"

#include "Platform/Vulkan/VulkanRenderer.h"

#include <thread>

GlobalEnvironment* g_pEnv;

namespace Lamp
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	void UpdateAssetManager(bool& running)
	{
		//while (running)
		//{
		//	g_pEnv->pAssetManager->Update();
		//}
	}

	Application* Application::s_pInstance = nullptr;

	Application::Application()
	{
		LP_PROFILE_FUNCTION();
		s_pInstance = this;
		g_pEnv = new GlobalEnvironment();

		g_pEnv->pAssetManager = new AssetManager();

		//Create the window
		WindowProps props;
		props.Height = 720;
		props.Width = 1280;
		props.IsVSync = false;
		props.Title = "Lamp";

		m_pWindow = Window::Create(props);
		m_pWindow->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_levelManager = CreateScope<LevelManager>();
		m_renderer = CreateScope<Renderer>();

		m_renderer->Initialize();
		RenderCommand::Initialize(m_renderer.get());

		AudioEngine::Initialize();
		Physics::Initialize();

		m_threadPool.AddThread("Update", LP_BIND_THREAD_FN(Application::UpdateApplication));

		//m_AssetManagerThread = std::thread(UpdateAssetManager, std::ref(m_running));

		//Setup the GUI system
		m_pImGuiLayer = ImGuiLayer::Create();
		PushOverlay(m_pImGuiLayer);
	}

	Application::~Application()
	{
		LP_PROFILE_FUNCTION();
		Physics::Shutdown();
		AudioEngine::Shutdown();

		RenderCommand::Shutdown();
		m_renderer->Shutdown();
		m_renderer.reset();

		ResourceCache::Shutdown();

		//m_AssetManagerThread.join();
		m_threadPool.JoinAll();

		delete g_pEnv->pAssetManager;
		delete g_pEnv;
	}

	void Application::UpdateApplication()
	{
		LP_PROFILE_THREAD("Update");

		while (m_running)
		{
			LP_PROFILE_FRAME("UpdateThread");

			m_updateReady = true;

			float time = (float)glfwGetTime();
			m_currentTimeStep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			m_updateFrameTime.Begin();

			AppUpdateEvent e(m_currentTimeStep);

			for (Layer* pLayer : m_LayerStack)
			{
				pLayer->OnEvent(e);
			}

			m_updateReady = false;

			{
				LP_PROFILE_SCOPE("Wait for render");

				while (!m_renderReady && m_running)
				{
				}
			}


			RenderCommand::SwapRenderBuffers();

			m_updateFrameTime.End();
		}
	}

	void Application::Run()
	{
		LP_PROFILE_THREAD("Render");

		while (m_running)
		{
			LP_PROFILE_FRAME("RenderThread");

			m_mainFrameTime.Begin();

			m_renderReady = true;
			 
			{
				LP_PROFILE_SCOPE("Wait for update");
				while (!m_updateReady)
				{
				}
			}

			m_renderReady = false;

			m_pWindow->GetSwapchain()->BeginFrame();

			AudioEngine::Update();

			for (Layer* pLayer : m_LayerStack)
			{
				pLayer->OnRender();
			}

			{
				LP_PROFILE_SCOPE("Application::UpdateImGui");

				m_pImGuiLayer->Begin();

				for (Layer* pLayer : m_LayerStack)
				{
					pLayer->OnImGuiRender(m_currentTimeStep);
				}

				m_pImGuiLayer->End();
			}

			m_pWindow->Update(m_currentTimeStep);

			m_mainFrameTime.End();
		}
	}

	void Application::OnEvent(Event& e)
	{
		LP_PROFILE_FUNCTION();
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		//Handle rest of events
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
			{
				break;
			}
		}
	}

	void Application::PushLayer(Layer* pLayer)
	{
		m_LayerStack.PushLayer(pLayer);
	}

	void Application::PushOverlay(Layer* pLayer)
	{
		m_LayerStack.PushOverlay(pLayer);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_running = false;
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 && e.GetHeight() == 0)
		{
			m_minimized = true;
			return false;
		}
		else
		{
			m_minimized = false;
		}
		m_pWindow->OnResize(e.GetWidth(), e.GetHeight());
		return false;
	}
}