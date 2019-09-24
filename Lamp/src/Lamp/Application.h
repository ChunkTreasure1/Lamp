#pragma once
#include "Window.h"
#include "Lamp/Rendering/Renderer.h"
#include "Lamp/Event/Event.h"

#include "Lamp/Event/ApplicationEvent.h"
#include "Lamp/Layer/Layer.h"
#include "Lamp/Layer/LayerStack.h"

//#include "ImGui/ImGuiLayer.h"

namespace Lamp
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* pLayer);
		void PushOverlay(Layer* pLayer);

		bool OnWindowClose(WindowCloseEvent& e);

		inline static Application& Get() { return *s_pInstance; }
		inline Window& GetWindow() { return *m_pWindow; }

	private:
		static Application* s_pInstance;

	private:
		std::unique_ptr<Window> m_pWindow;
		//ImGuiLayer* m_pImGuiLayer;
		LayerStack m_LayerStack;

		bool m_Running = true;
		float m_LastFrameTime = 0.f;
	};
	
	static Application* CreateApplication();
}