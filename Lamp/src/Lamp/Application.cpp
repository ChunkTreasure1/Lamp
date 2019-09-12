#include "lppch.h"
#include "Application.h"

namespace Lamp 
{
	Application::Application()
	{
	}


	Application::~Application()
	{
		m_Running = false;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			//Updates all the layers
			for (Layer* pLayer : m_LayerStack)
			{
				pLayer->Update();
			}
		}
	}

	void Application::PushLayer(Layer* pLayer)
	{
		m_LayerStack.PushLayer(pLayer);
		pLayer->OnAttach();
	}
	void Application::PushOverlay(Layer* pOverlay)
	{
		m_LayerStack.PushOverlay(pOverlay);
		pOverlay->OnAttach();
	}
}