#include "lppch.h"
#include "Application.h"

#include "imgui.h"

#include "Lamp/Rendering/RenderCommand.h"
#include "Lamp/Audio/AudioEngine.h"
#include "Lamp/Physics/Physics.h"
#include "Lamp/AssetSystem/AssetManager.h"
#include "Lamp/Objects/Entity/ComponentInclude.h"
#include "Lamp/ImGui/ImGuiLayer.h"
#include "Lamp/Core/CoreLogger.h"
#include "Lamp/Rendering/Swapchain.h"

#include <thread>

GlobalEnvironment* g_pEnv;

namespace Lamp
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	void UpdateAssetManager(bool& running)
	{
		while (running)
		{
			g_pEnv->pAssetManager->Update();
		}
	}

	Application* Application::s_pInstance = nullptr;

	Application::Application()
	{
		LP_PROFILE_FUNCTION();
		s_pInstance = this;
		g_pEnv = new GlobalEnvironment();

		g_pEnv->pAssetManager = new AssetManager();

		//Create the window
		WindowProps props;
		props.Height = 720;
		props.Width = 1280;
		props.IsVSync = false;
		props.Title = "Lamp";

		m_pWindow = Window::Create(props);
		m_pWindow->SetEventCallback(BIND_EVENT_FN(OnEvent));

		Renderer::Initialize();
		//AudioEngine::Initialize();
		//Physics::Initialize();

		m_AssetManagerThread = std::thread(UpdateAssetManager, std::ref(m_Running));

		//Setup the GUI system
		//m_pImGuiLayer = new ImGuiLayer();
		//PushOverlay(m_pImGuiLayer);
	}

	Application::~Application()
	{
		LP_PROFILE_FUNCTION();
		//AudioEngine::Shutdown();
		//Renderer::Shutdown();

		m_AssetManagerThread.join();

		g_pEnv->pLevel->Shutdown(); // TODO: this needs to be fixed

		delete g_pEnv->pAssetManager;
		delete g_pEnv;
	}

	void Application::Run()
	{
		while (m_Running)
		{
			LP_PROFILE_SCOPE("Application::Run::TotalLoop");
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			m_FrameTime.Begin();

			m_pWindow->GetSwapchain()->BeginFrame();
			Renderer::Begin(nullptr);

			//AudioEngine::Update();
			////Load 
			//{
			//	LP_PROFILE_SCOPE("Application::UpdateLayers");
			//	AppUpdateEvent e(timestep);

			//	for (Layer* pLayer : m_LayerStack)
			//	{
			//		pLayer->OnEvent(e);
			//	}
			//}

			//{
			//	LP_PROFILE_SCOPE("Application::UpdateImGui");

			//	m_pImGuiLayer->Begin();

			//	for (Layer* pLayer : m_LayerStack)
			//	{
			//		pLayer->OnImGuiRender(timestep);
			//	}

			//	m_pImGuiLayer->End();
			//}

			m_pWindow->Update(timestep);

			m_FrameTime.End();
		}
	}

	void Application::OnEvent(Event& e)
	{
		LP_PROFILE_FUNCTION();
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(OnWindowResize));

		////Handle rest of events
		//for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		//{
		//	(*--it)->OnEvent(e);
		//	if (e.Handled)
		//	{
		//		break;
		//	}
		//}
	}

	void Application::PushLayer(Layer* pLayer)
	{
		m_LayerStack.PushLayer(pLayer);
	}

	void Application::PushOverlay(Layer* pLayer)
	{
		m_LayerStack.PushOverlay(pLayer);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return false;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
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

		RenderCommand::SetViewport(0, 0, e.GetWidth(), e.GetHeight());
		return false;
	}
}