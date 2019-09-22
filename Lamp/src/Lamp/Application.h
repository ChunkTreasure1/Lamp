#pragma once
#include "Window.h"
#include "Lamp/Rendering/Renderer.h"

namespace Lamp
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	public:
		static Renderer* s_pRenderer;

	private:
		static const bool m_sRunning = true;

	private:
		Window* m_pWindow;

		float m_Time = 0;
		float m_FPS = 0;
	};
	
	static Application* CreateApplication();
}