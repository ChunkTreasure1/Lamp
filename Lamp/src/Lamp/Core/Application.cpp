#include "lppch.h"
#include "Application.h"

#include "imgui.h"

#include "Lamp/Physics/PhysicsEngine.h"
#include "Lamp/Objects/ObjectLayer.h"
#include "Lamp/Rendering/RenderCommand.h"
#include "Lamp/Audio/AudioEngine.h"

#include "Lamp/Objects/Brushes/BrushManager.h"
#include "Lamp/Objects/Entity/Base/EntityManager.h"

#include "CoreLogger.h"

GlobalEnvironment* g_pEnv;

namespace Lamp
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_pInstance = nullptr;

	Application::Application()
	{
		LP_PROFILE_FUNCTION();
		s_pInstance = this;
		g_pEnv = new GlobalEnvironment();
		g_pEnv->pRenderUtils = new RenderUtils();

		g_pEnv->pObjectLayerManager = new ObjectLayerManager();
		g_pEnv->pEntityManager = new EntityManager();
		g_pEnv->pBrushManager = new BrushManager();

		//Create the window
		WindowProps props;
		props.Height = 720;
		props.Width = 1280;
		props.IsVSync = false;
		props.Title = "Lamp";

		m_pWindow = Window::Create(props);
		m_pWindow->SetEventCallback(BIND_EVENT_FN(OnEvent));

		Renderer::Initialize();
		AudioEngine::Initialize();

		m_pPhysicsEngine = new PhysicsEngine();
		m_pPhysicsEngine->Initialize();

		//Setup the GUI system
		m_pImGuiLayer = new ImGuiLayer();
		PushOverlay(m_pImGuiLayer);
	}

	Application::~Application()
	{
		LP_PROFILE_FUNCTION();
		m_pPhysicsEngine->Shutdown();
		AudioEngine::Shutdown();
		Renderer::Shutdown();

		delete g_pEnv;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			LP_PROFILE_SCOPE("Application::Run::TotalLoop");
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			m_FrameTime.Begin();

			if (m_IsSimulating)
			{
				LP_PROFILE_SCOPE("Application::Run::Physics")
				m_pPhysicsEngine->Simulate();
			}
			AudioEngine::Update();

			//Update the application layers

			if (!m_Minimized)
			{
				AppUpdateEvent updateEvent(timestep);
				OnEvent(updateEvent);

				{
					LP_PROFILE_SCOPE("Application::Run::ObjectUpdate");
					ObjectLayerManager::Get()->OnEvent(updateEvent);
				}
			}

			m_pImGuiLayer->Begin();

			for (Layer* pLayer : m_LayerStack)
			{
				pLayer->OnImGuiRender(timestep);
			}

			m_pImGuiLayer->End();
			m_pWindow->Update(timestep);
		
			m_FrameTime.End();
		}
	}

	void Application::OnEvent(Event & e)
	{
		LP_PROFILE_FUNCTION();
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		////Handle rest of events
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
			{
				break;
			}
		}
	}

	void Application::PushLayer(Layer * pLayer)
	{
		m_LayerStack.PushLayer(pLayer);
	}

	void Application::PushOverlay(Layer * pLayer)
	{
		m_LayerStack.PushOverlay(pLayer);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent & e)
	{
		if (e.GetWidth() == 0 && e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}
		else
		{
			m_Minimized = false;
		}

		RenderCommand::SetViewport(0, 0, e.GetWidth(), e.GetHeight());
		return false;
	}
}