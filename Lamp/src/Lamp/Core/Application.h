#pragma once
#include "Lamp/Event/Event.h"
#include "Lamp/Event/ApplicationEvent.h"

#include "Lamp/Core/ThreadPool.h"

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

		inline const ThreadPool& GetThreadPool() { return m_threadPool; }
		inline Window& GetWindow() { return *m_pWindow; }

		inline const FrameTime& GetMainFrameTime() { return m_mainFrameTime; }
		inline const FrameTime& GetUpdateFrameTime() { return m_updateFrameTime; }

	private:
		void UpdateApplication();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

	private:
		static Application* s_pInstance;

	private:
		Scope<Window> m_pWindow;
		Scope<LevelManager> m_levelManager;
		Scope<Renderer> m_renderer;
		Scope<Renderer2D> m_renderer2D;

		ImGuiLayer* m_pImGuiLayer;
		LayerStack m_LayerStack;

		PhysicsEngine* m_pPhysicsEngine = nullptr;

		std::atomic_bool m_running = true;
		std::atomic_bool m_minimized = false;

		std::thread m_updateThread;
		std::atomic_bool m_renderReady = false;
		std::atomic_bool m_updateReady = true;

		float m_LastFrameTime = 0.f;
		std::atomic<Timestep> m_currentTimeStep;

		FrameTime m_mainFrameTime;
		FrameTime m_updateFrameTime;

		ThreadPool m_threadPool;
		std::thread m_AssetManagerThread;
	};
	
	static Application* CreateApplication();
}