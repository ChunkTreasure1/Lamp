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
#include "Lamp/Rendering/Renderer2D.h"

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

		m_window = Window::Create(props);
		m_window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		m_levelManager = CreateScope<LevelManager>();
		m_renderer = CreateScope<Renderer>();
		m_renderer2D = CreateScope<Renderer2D>();

		m_renderer->Initialize();
		m_renderer2D->Initialize();
		RenderCommand::Initialize(m_renderer.get(), m_renderer2D.get());

		AudioEngine::Initialize();
		Physics::Initialize();

		//Setup the GUI system
		m_pImGuiLayer = ImGuiLayer::Create();
		PushOverlay(m_pImGuiLayer);
	
		m_renderThread = std::thread(std::bind(&Application::RenderThread, this));
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

		delete g_pEnv->pAssetManager;
		delete g_pEnv;
	}

	void Application::Run()
	{
		LP_PROFILE_THREAD("Main");

		while (m_running)
		{
			LP_PROFILE_FRAME("Main");

			m_mainFrameTime.Begin();

			float time = (float)glfwGetTime();
			m_currentTimeStep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			AppUpdateEvent e(m_currentTimeStep);

			for (Layer* pLayer : m_LayerStack)
			{
				pLayer->OnEvent(e);
			}

			AudioEngine::Update();
			
			{
				std::unique_lock<std::mutex> lock(m_renderThreadMutex);
				m_renderCondition.wait(lock, [=] 
					{
						LP_PROFILE_SCOPE("Wait for render");
						return m_renderReady; 
					});
			}

			{
				std::lock_guard<std::mutex> lock(m_renderThreadMutex);
				m_updateReady = true;
			}
			m_renderCondition.notify_all();

			RenderCommand::SwapRenderBuffers();
			m_renderReady = false;

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

	void Application::RenderThread()
	{
		LP_PROFILE_THREAD("Render");

		while (m_running)
		{
			LP_PROFILE_FRAME("Render");

			std::unique_lock<std::mutex> lock(m_renderThreadMutex);
			m_renderCondition.wait(lock, [=]() 
				{
					LP_PROFILE_SCOPE("Wait for update");
					return m_updateReady; 
				});

			m_window->GetSwapchain()->BeginFrame();

			//Processing
			//Dispatch

			{
				LP_PROFILE_SCOPE("OnRender");
				for (Layer* pLayer : m_LayerStack)
				{
					pLayer->OnRender();
				}
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

			m_renderReady = true;
			m_updateReady = false;

			m_window->Update(m_currentTimeStep);
		
			lock.unlock();
			m_renderCondition.notify_all();
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
		m_window->OnResize(e.GetWidth(), e.GetHeight());
		return false;
	}
}