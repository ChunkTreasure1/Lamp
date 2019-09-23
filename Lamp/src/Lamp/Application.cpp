#include "lppch.h"
#include "Application.h"

#include "Lamp/Event/ApplicationEvent.h"

namespace Lamp
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_pInstance = nullptr;
	Renderer* Application::s_pRenderer = nullptr;

	Application::Application()
	{
		s_pInstance = this;

		//Create the window
		m_pWindow = new Window();
		m_pWindow->SetEventCallback(BIND_EVENT_FN(OnEvent));

		s_pRenderer = new Renderer(m_pWindow);
	}

	Application::~Application()
	{
		delete m_pWindow;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			s_pRenderer->Draw();
		}
	}

	void Application::OnEvent(Event & e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		LP_CORE_TRACE("{0}", e);

		//Handle rest of events
	}

	bool Application::OnWindowClose()
	{
		m_Running = false;
		return true;
	}
}