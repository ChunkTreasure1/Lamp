#include "lppch.h"
#include "SandboxLayer.h"

#include "Windows/MeshImporterPanel.h"
#include "Windows/GraphKey.h"
#include "Windows/MaterialEditor.h"
#include "Windows/LayerViewPanel.h"
#include "Windows/EnvironmentEditorPanel.h"
#include "Windows/TerrainEditorPanel.h"
#include "Windows/RenderGraphPanel.h"

#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Rendering/RenderCommand.h>
#include <Lamp/Rendering/Renderer2D.h>

#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/Core/Application.h>
#include <Lamp/Core/Game.h>
#include <Lamp/Core/Time/ScopedTimer.h>
#include <Lamp/World/Terrain.h>


namespace Sandbox
{
	using namespace Lamp;

	SandboxLayer::SandboxLayer()
		: Layer("Sandbox"), m_dockspaceId(0), m_physicsIcon("engine/textures/ui/physicsIcon/LampPhysicsAnim1.png", 30)
	{
		g_pEnv->isEditor = true;
		m_iconPlay = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/PlayIcon.png");
		m_iconStop = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/StopIcon.png");
		m_iconStop = ResourceCache::GetAsset<Texture2D>("engine/textures/iceland_heightmap.png");
		ResourceCache::GetAsset<Texture2D>("engine/textures/default/defaultTexture.png");
		//ResourceCache::GetAsset<Texture2D>("assets/textures/TeddyTextures/teddy_albedo.ktx2");

		LevelManager::Get()->Load("assets/levels/testLevel/data.level");

		m_sandboxController = CreateRef<SandboxController>();

		m_pWindows.emplace_back(new MeshImporterPanel("Mesh Importer"));
		m_pWindows.emplace_back(new GraphKey("Visual Scripting"));
		m_pWindows.emplace_back(new MaterialEditor("Material Editor"));
		m_pWindows.emplace_back(new LayerViewPanel("Layer View", &m_pSelectedObject));
		m_pWindows.emplace_back(new EnvironmentEditorPanel("Environment Panel"));
		m_pWindows.emplace_back(new AssetBrowserPanel());
		m_pWindows.emplace_back(new CreatePanel(m_pSelectedObject));
		m_pWindows.emplace_back(new TerrainEditorPanel("Terrain Editor"));
		m_pWindows.emplace_back(new RenderGraphPanel("Render Graph"));

		Application::Get().GetWindow().Maximize();

		SetupFromConfig();
	}

	SandboxLayer::~SandboxLayer()
	{
		for (auto p : m_pWindows)
		{
			delete p;
		}
		m_pWindows.clear();
	}

	bool SandboxLayer::OnUpdate(AppUpdateEvent& e)
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
			m_physicsIcon.OnEvent(e);
		}

		return false;
	}

	void SandboxLayer::OnImGuiRender(Timestep ts)
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

		ImGuiUpdateEvent e;
		OnEvent(e);

		m_uiTotalTime = timerTotal.GetTime();
	}

	void SandboxLayer::OnEvent(Event& e)
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
		dispatcher.Dispatch<MouseMovedEvent>(LP_BIND_EVENT_FN(SandboxLayer::OnMouseMoved));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(SandboxLayer::OnUpdate));
		dispatcher.Dispatch<WindowCloseEvent>(LP_BIND_EVENT_FN(SandboxLayer::OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(LP_BIND_EVENT_FN(SandboxLayer::OnKeyPressed));
		dispatcher.Dispatch<ImGuiBeginEvent>(LP_BIND_EVENT_FN(SandboxLayer::OnImGuiBegin));
		dispatcher.Dispatch<EditorViewportSizeChangedEvent>(LP_BIND_EVENT_FN(SandboxLayer::OnViewportSizeChanged));
		dispatcher.Dispatch<EditorObjectSelectedEvent>(LP_BIND_EVENT_FN(SandboxLayer::OnObjectSelected));
	}

	void SandboxLayer::OnRender()
	{
		LP_PROFILE_FUNCTION();

		RenderCommand::Begin(m_sandboxController->GetCameraController()->GetCamera());

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

		RenderCommand::End();
	}

	bool SandboxLayer::OnKeyPressed(KeyPressedEvent& e)
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

	bool SandboxLayer::OnImGuiBegin(ImGuiBeginEvent& e)
	{
		ImGuizmo::BeginFrame();
		return true;
	}

	bool SandboxLayer::OnViewportSizeChanged(Lamp::EditorViewportSizeChangedEvent& e)
	{
		uint32_t width = e.GetWidth();
		uint32_t height = e.GetHeight();

		m_sandboxController->GetCameraController()->UpdateProjection(width, height);

		return false;
	}

	bool SandboxLayer::OnObjectSelected(Lamp::EditorObjectSelectedEvent& e)
	{
		if (m_pSelectedObject)
		{
			m_pSelectedObject->SetIsSelected(false);
		}

		m_pSelectedObject = e.GetObject();
		m_pSelectedObject->SetIsSelected(true);
		return false;
	}

	void SandboxLayer::GetInput()
	{
		if (m_perspectiveFocused)
		{
			if (Input::IsKeyPressed(LP_KEY_1))
			{
				m_imGuizmoOperation = ImGuizmo::TRANSLATE;
			}

			if (Input::IsKeyPressed(LP_KEY_2))
			{
				m_imGuizmoOperation = ImGuizmo::ROTATE;
			}

			if (Input::IsKeyPressed(LP_KEY_3))
			{
				m_imGuizmoOperation = ImGuizmo::SCALE;
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

	void SandboxLayer::OnLevelPlay()
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

	void SandboxLayer::OnLevelStop()
	{
		m_SceneState = SceneState::Edit;
		m_pSelectedObject = nullptr;

		m_runtimeLevel->OnRuntimeEnd();

		LevelManager::Get()->SetActive(m_editLevel);
		m_editLevel->SetIsPlaying(false);

		m_runtimeLevel = nullptr;
		m_pGame = nullptr;
	}

	void SandboxLayer::OnSimulationStart()
	{
		m_SceneState = SceneState::Simulating;
		m_pSelectedObject = nullptr;

		m_editLevel = LevelManager::GetActive();
		m_runtimeLevel = CreateRef<Level>(*m_editLevel);

		LevelManager::Get()->SetActive(m_runtimeLevel);
		m_runtimeLevel->SetIsPlaying(true);
		m_runtimeLevel->OnSimulationStart();
	}

	void SandboxLayer::OnSimulationStop()
	{
		m_SceneState = SceneState::Edit;
		m_pSelectedObject = nullptr;

		m_runtimeLevel->OnSimulationEnd();
		LevelManager::Get()->SetActive(m_editLevel);
		m_editLevel->SetIsPlaying(false);

		m_runtimeLevel = nullptr;
	}
}