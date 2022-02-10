#include "lppch.h"
#include "Sandbox.h"

#include "Windows/MeshImporterPanel.h"
#include "Windows/GraphKey.h"
#include "Windows/MaterialEditor.h"
#include "Windows/LayerViewPanel.h"
#include "Windows/EnvironmentEditorPanel.h"

#include <Lamp/Rendering/Shadows/PointShadowBuffer.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Rendering/Renderer2D.h>
#include <Lamp/Rendering/RenderPipeline.h>

#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/Core/Application.h>
#include <Lamp/Core/Game.h>
#include <Lamp/Core/Time/ScopedTimer.h>
#include <Lamp/World/Terrain.h>


namespace Sandbox
{
	using namespace Lamp;

	Sandbox::Sandbox()
		: Layer("Sandbox"), m_DockspaceID(0), m_PhysicsIcon("engine/textures/ui/physicsIcon/LampPhysicsAnim1.png", 30),
		m_createPanel(m_pSelectedObject)
	{
		g_pEnv->isEditor = true;
		m_IconPlay = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/PlayIcon.png");
		m_IconStop = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/StopIcon.png");
		ResourceCache::GetAsset<Texture2D>("engine/textures/default/defaultTexture.png");
		
		//ResourceCache::GetAsset<Texture2D>("assets/textures/TeddyTextures/teddy_albedo.ktx2");

		LevelManager::Get()->Load("assets/levels/testLevel/data.level");

		m_sandboxController = CreateRef<SandboxController>();

		m_pWindows.push_back(new MeshImporterPanel("Mesh Importer"));
		m_pWindows.push_back(new GraphKey("Visual Scripting"));
		m_pWindows.push_back(new MaterialEditor("Material Editor"));
		m_pWindows.push_back(new LayerViewPanel("Layer View", &m_pSelectedObject));
		m_pWindows.push_back(new EnvironmentEditorPanel("Environment Panel"));

		Application::Get().GetWindow().Maximize();

		SetupFromConfig();
	}

	Sandbox::~Sandbox()
	{
		for (auto p : m_pWindows)
		{
			delete p;
		}
		m_pWindows.clear();
	}

	bool Sandbox::OnUpdate(AppUpdateEvent& e)
	{
		LP_PROFILE_FUNCTION();

		m_sandboxController->Update(e.GetTimestep());

		GetInput();

		if (LevelManager::GetActive())
		{
			LP_PROFILE_SCOPE("Sandbox::Update::LevelUpdate");

			auto level = LevelManager::GetActive();

			switch (m_SceneState)
			{
				case SceneState::Edit:
				{
					level->UpdateEditor(e.GetTimestep(), m_sandboxController->GetCameraController()->GetCamera());
					break;
				}
				case SceneState::Play:
				{
					level->UpdateRuntime(e.GetTimestep(), m_sandboxController->GetCameraController()->GetCamera());
					break;
				}
				case SceneState::Simulating:
				{
					level->UpdateSimulation(e.GetTimestep());
					break;
				}
			}
		}

		{
			LP_PROFILE_SCOPE("Sandbox3D::Update::UIUpdate");
			m_PhysicsIcon.OnEvent(e);
		}

		{
			LP_PROFILE_SCOPE("Sandbox3D::Update::WindowRendering");
			for (auto pWindow : m_pWindows)
			{
				if (pWindow->GetIsOpen())
				{
					for (const auto& pFunc : pWindow->GetRenderFuncs())
					{
						pFunc();
					}
				}
			}
		}

		return false;
	}

	void Sandbox::OnImGuiRender(Timestep ts)
	{
		LP_PROFILE_FUNCTION();
		ScopedTimer timerTotal{};

		CreateDockspace();

		UpdateProperties();
		UpdatePerspective();
		UpdateLogTool();
		UpdateLevelSettings();
		UpdateRenderingSettings();
		UpdateToolbar();
		UpdateStatistics();

		{
			ScopedTimer timer{};
			m_assetManager.OnImGuiRender();
			m_assetManagerTime = timer.GetTime();
		}

		{
			ScopedTimer timer{};
			m_createPanel.OnImGuiRender();

			m_createPanelTime = timer.GetTime();
		}

		ImGuiUpdateEvent e;
		OnEvent(e);

		m_uiTotalTime = timerTotal.GetTime();
	}

	void Sandbox::OnEvent(Event& e)
	{
		if (m_SceneState == SceneState::Play && m_pGame)
		{
			m_pGame->OnEvent(e);
		}

		for (auto pWindow : m_pWindows)
		{
			if (pWindow->GetIsOpen())
			{
				pWindow->OnEvent(e);
			}
		}

		m_sandboxController->OnEvent(e);
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseMovedEvent>(LP_BIND_EVENT_FN(Sandbox::OnMouseMoved));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(Sandbox::OnUpdate));
		dispatcher.Dispatch<WindowCloseEvent>(LP_BIND_EVENT_FN(Sandbox::OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(LP_BIND_EVENT_FN(Sandbox::OnKeyPressed));
		dispatcher.Dispatch<ImGuiBeginEvent>(LP_BIND_EVENT_FN(Sandbox::OnImGuiBegin));
		dispatcher.Dispatch<EditorViewportSizeChangedEvent>(LP_BIND_EVENT_FN(Sandbox::OnViewportSizeChanged));
		dispatcher.Dispatch<EditorObjectSelectedEvent>(LP_BIND_EVENT_FN(Sandbox::OnObjectSelected));
	}

	void Sandbox::OnRender()
	{
		Renderer::Begin(m_sandboxController->GetCameraController()->GetCamera());

		if (LevelManager::GetActive())
		{
			auto level = LevelManager::GetActive();

			switch (m_SceneState)
			{
				case SceneState::Edit:
				{
					level->RenderEditor();
					break;
				}

				case SceneState::Play:
				{
					level->RenderRuntime();
					break;
				}

				case SceneState::Simulating:
				{
					level->RenderSimulation();
					break;
				}
			}
		}

		Renderer::End();
	}

	bool Sandbox::OnKeyPressed(KeyPressedEvent& e)
	{
		//Shortcuts
		if (e.GetRepeatCount() > 0)
		{
			return false;
		}

		switch (e.GetKeyCode())
		{
			case LP_KEY_S:
			{
				bool control = Input::IsKeyPressed(LP_KEY_LEFT_CONTROL) || Input::IsKeyPressed(LP_KEY_RIGHT_CONTROL);
				bool shift = Input::IsKeyPressed(LP_KEY_LEFT_SHIFT) || Input::IsKeyPressed(LP_KEY_RIGHT_SHIFT);

				if (control && shift)
				{
					SaveLevelAs();
				}
				else if (control && !shift)
				{

					if (LevelManager::GetActive()->Path.empty())
					{
						SaveLevelAs();
						break;
					}
					else
					{
						g_pEnv->pAssetManager->SaveAsset(LevelManager::GetActive());
					}
				}
				break;
			}

			case LP_KEY_N:
			{
				bool control = Input::IsKeyPressed(LP_KEY_LEFT_CONTROL) || Input::IsKeyPressed(LP_KEY_RIGHT_CONTROL);

				if (control)
				{
					NewLevel();
				}
				break;
			}

			case LP_KEY_O:
			{
				bool control = Input::IsKeyPressed(LP_KEY_LEFT_CONTROL) || Input::IsKeyPressed(LP_KEY_RIGHT_CONTROL);
				if (control)
				{
					OpenLevel();
				}
				break;
			}

			case LP_KEY_Z:
			{
				bool control = Input::IsKeyPressed(LP_KEY_LEFT_CONTROL) || Input::IsKeyPressed(LP_KEY_RIGHT_CONTROL);
				if (control)
				{
					Undo();
				}
				break;
			}

			case LP_KEY_Y:
			{
				bool control = Input::IsKeyPressed(LP_KEY_LEFT_CONTROL) || Input::IsKeyPressed(LP_KEY_RIGHT_CONTROL);
				if (control)
				{
					Redo();
				}
				break;
			}

			case LP_KEY_G:
			{
				bool control = Input::IsKeyPressed(LP_KEY_LEFT_CONTROL) || Input::IsKeyPressed(LP_KEY_RIGHT_CONTROL);
				if (control)
				{
					if (m_SceneState == SceneState::Edit)
					{
						OnLevelPlay();
					}
					else if (m_SceneState == SceneState::Play)
					{
						OnLevelStop();
					}
				}

				break;
			}
		}

		return false;
	}

	bool Sandbox::OnImGuiBegin(ImGuiBeginEvent& e)
	{
		ImGuizmo::BeginFrame();
		return true;
	}

	bool Sandbox::OnViewportSizeChanged(Lamp::EditorViewportSizeChangedEvent& e)
	{
		uint32_t width = e.GetWidth();
		uint32_t height = e.GetHeight();

		m_sandboxController->GetCameraController()->UpdateProjection(width, height);

		return false;
	}

	bool Sandbox::OnObjectSelected(Lamp::EditorObjectSelectedEvent& e)
	{
		if (m_pSelectedObject)
		{
			m_pSelectedObject->SetIsSelected(false);
		}

		m_pSelectedObject = e.GetObject();
		m_pSelectedObject->SetIsSelected(true);
		return false;
	}

	void Sandbox::GetInput()
	{
		if (Input::IsMouseButtonPressed(0))
		{
			m_MousePressed = true;
		}
		else if (Input::IsMouseButtonReleased(0))
		{
			m_MousePressed = false;
		}

		if (m_perspectiveFocused)
		{
			if (Input::IsKeyPressed(LP_KEY_1))
			{
				m_ImGuizmoOperation = ImGuizmo::TRANSLATE;
			}

			if (Input::IsKeyPressed(LP_KEY_2))
			{
				m_ImGuizmoOperation = ImGuizmo::ROTATE;
			}

			if (Input::IsKeyPressed(LP_KEY_3))
			{
				m_ImGuizmoOperation = ImGuizmo::SCALE;
			}

			if (Input::IsKeyPressed(LP_KEY_DELETE))
			{
				if (m_pSelectedObject)
				{
					m_pSelectedObject->Destroy();
					m_pSelectedObject = nullptr;
				}
			}
		}
	}

	void Sandbox::OnLevelPlay()
	{
		m_SceneState = SceneState::Play;
		m_pSelectedObject = nullptr;

		m_editLevel = LevelManager::GetActive();
		m_runtimeLevel = CreateRef<Level>(*m_editLevel);

		LevelManager::Get()->SetActive(m_runtimeLevel);
		m_runtimeLevel->OnRuntimeStart();
		m_runtimeLevel->SetIsPlaying(true);

		m_pGame = CreateScope<Game>();
		m_pGame->OnStart();
	}

	void Sandbox::OnLevelStop()
	{
		m_SceneState = SceneState::Edit;
		m_pSelectedObject = nullptr;

		m_runtimeLevel->OnRuntimeEnd();

		LevelManager::Get()->SetActive(m_editLevel);
		m_editLevel->SetIsPlaying(false);

		m_runtimeLevel = nullptr;
		m_pGame = nullptr;
	}

	void Sandbox::OnSimulationStart()
	{
		m_SceneState = SceneState::Simulating;
		m_pSelectedObject = nullptr;

		m_editLevel = LevelManager::GetActive();
		m_runtimeLevel = CreateRef<Level>(*m_editLevel);

		LevelManager::Get()->SetActive(m_runtimeLevel);
		m_runtimeLevel->SetIsPlaying(true);
		m_runtimeLevel->OnSimulationStart();
	}

	void Sandbox::OnSimulationStop()
	{
		m_SceneState = SceneState::Edit;
		m_pSelectedObject = nullptr;

		m_runtimeLevel->OnSimulationEnd();
		LevelManager::Get()->SetActive(m_editLevel);
		m_editLevel->SetIsPlaying(false);

		m_runtimeLevel = nullptr;
	}
}