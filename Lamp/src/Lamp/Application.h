#pragma once
#include "Window.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Event/Event.h"
#include "Lamp/Event/ApplicationEvent.h"

namespace Lamp
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);

		bool OnWindowClose(WindowCloseEvent& e);

		inline static Application& Get() { return *s_pInstance; }
		inline Window& GetWindow() { return *m_pWindow; }

	public:
		static Renderer* s_pRenderer;

	private:
		static Application* s_pInstance;

	private:
		Window* m_pWindow;

		bool m_Running = true;
		float m_Time = 0;
		float m_FPS = 0;
	};
	
	static Application* CreateApplication();
}