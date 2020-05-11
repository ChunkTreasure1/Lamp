#include "lppch.h"
#include "Application.h"

#include "imgui.h"

namespace Lamp
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_pInstance = nullptr;

	Application::Application()
	{
		s_pInstance = this;

		//Create the window
		m_pWindow = std::unique_ptr<Window>(new Window());
		m_pWindow->SetEventCallback(BIND_EVENT_FN(OnEvent));

		Renderer::Initialize();

		m_pImGuiLayer = new ImGuiLayer();
		PushOverlay(m_pImGuiLayer);
	}

	Application::~Application()
	{
		Renderer::Shutdown();
	}

	void Application::Run()
	{
		while (m_Running)
		{
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				//Lamp::Physics::CheckCollisions();
				for (Layer* layer : m_LayerStack)
				{
					layer->Update(timestep);
				}
			}

			m_pImGuiLayer->Begin();

			for (Layer* pLayer : m_LayerStack)
			{
				pLayer->OnImGuiRender(timestep);
			}

			m_pImGuiLayer->End();
			m_pWindow->Update(timestep);
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
		return true;
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

		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());
		return false;
	}
	
	void Application::OnItemClicked(File & file)
	{
		for (Layer* pLayer : m_LayerStack)
		{
			pLayer->OnItemClicked(file);
		}
	}
}