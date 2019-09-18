#pragma once
#include "Lamp/Rendering/Window.h"

namespace Lamp
{
	class Application
	{
	public:
		Application();
		~Application();

		void Run();

	private:
		bool m_Running = true;

	private:
		static Application* s_pInstance;
		Window* m_pWindow;
	};

	Application* CreateApplication();
}