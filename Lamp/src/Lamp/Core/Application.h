#pragma once
#include "Lamp/Event/Event.h"
#include "Lamp/Event/ApplicationEvent.h"

#include "Lamp/Core/Threading/ThreadPool.h"

#include "Lamp/Layer/Layer.h"
#include "Lamp/Layer/LayerStack.h"
#include "Lamp/Level/LevelManager.h"

#include "Lamp/Input/FileSystem.h"

#include "Window.h"
#include "Time/FrameTime.h"

namespace Lamp
{
	class PhysicsEngine;
	class ImGuiLayer;
	class Renderer;
	class Renderer2D;

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
		void OnEvent(Event& e);

		void PushLayer(Layer* pLayer);
		void PushOverlay(Layer* pLayer);

		inline static Application& Get() { return *s_pInstance; }

		inline Window& GetWindow() { return *m_window; }

		inline const FrameTime& GetMainFrameTime() { return m_mainFrameTime; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		static Application* s_pInstance;

	private:
		Scope<Window> m_window;
		Scope<LevelManager> m_levelManager;
		Scope<Renderer> m_renderer;
		Scope<Renderer2D> m_renderer2D;

		ImGuiLayer* m_pImGuiLayer;
		LayerStack m_LayerStack;

		PhysicsEngine* m_pPhysicsEngine = nullptr;

		bool m_running = true;
		bool m_minimized = false;

		float m_LastFrameTime = 0.f;
		Timestep m_currentTimeStep;

		FrameTime m_mainFrameTime;

		ThreadPool m_threadPool;
		std::thread m_AssetManagerThread;
	};
	
	static Application* CreateApplication();
}