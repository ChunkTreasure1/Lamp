#pragma once
#include "Lamp/Layer/LayerStack.h"

namespace Lamp
{
	class Application
	{
	public:
		Application();
		~Application();

		void Run();
		void PushLayer(Layer* pLayer);
		void PushOverlay(Layer* pOverlay);

	private:
		LayerStack m_LayerStack;
		bool m_Running = true;

	private:
		static Application* s_pInstance;
	};

	Application* CreateApplication();
}