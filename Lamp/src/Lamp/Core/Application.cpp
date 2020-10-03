#include "lppch.h"
#include "Application.h"

#include "imgui.h"

#include "Lamp/Physics/PhysicsEngine.h"
#include "Lamp/Objects/ObjectLayer.h"
#include "Lamp/Rendering/RenderCommand.h"
#include "Lamp/Audio/AudioEngine.h"

#include "CoreLogger.h"

GlobalEnvironment* g_pEnv;

namespace Lamp
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_pInstance = nullptr;

	Application::Application()
	{
		s_pInstance = this;
		g_pEnv = new GlobalEnvironment();

		//Create the window
		m_pWindow = Window::Create();
		m_pWindow->SetEventCallback(BIND_EVENT_FN(OnEvent));

		Renderer::Initialize();
		AudioEngine::Initialize();

		//Setup the GUI system
		//m_pImGuiLayer = new ImGuiLayer();
		//PushOverlay(m_pImGuiLayer);
	}

	Application::~Application()
	{
		AudioEngine::Shutdown();
		Renderer::Shutdown();

		delete g_pEnv;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			m_FrameTime.Begin();

			//PhysicsEngine::Get()->Simulate(timestep);
			//PhysicsEngine::Get()->HandleCollisions();
			AudioEngine::Update();

			//AppUpdateEvent updateEvent(timestep);
			//ObjectLayerManager::Get()->OnEvent(updateEvent);

			//Update the application layers

			if (!m_Minimized)
			{
				AppUpdateEvent updateEvent(timestep);
				OnEvent(updateEvent);
			}

			/*		m_pImGuiLayer->Begin();

					for (Layer* pLayer : m_LayerStack)
					{
						pLayer->OnImGuiRender(timestep);
					}

					m_pImGuiLayer->End();*/
			m_pWindow->Update(timestep);
		
			m_FrameTime.End();
		}
	}

	void Application::OnEvent(Event & e)
	{
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