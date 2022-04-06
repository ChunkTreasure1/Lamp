#include "lppch.h"
#include "SandboxLayer.h"

#include "Windows/MeshImporterPanel.h"
#include "Windows/GraphKey.h"
#include "Windows/MaterialEditorPanel.h"
#include "Windows/LayerViewPanel.h"
#include "Windows/EnvironmentEditorPanel.h"
#include "Windows/TerrainEditorPanel.h"
#include "Windows/RenderGraphPanel.h"

#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Rendering/RenderCommand.h>
#include <Lamp/Rendering/Renderer2D.h>

#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/AssetSystem/MeshImporter/MeshImporter.h>
#include <Lamp/World/Terrain.h>

#include <Lamp/Core/Application.h>
#include <Lamp/Core/Game.h>
#include <Lamp/Core/Time/ScopedTimer.h>



namespace Sandbox
{
	using namespace Lamp;

	SandboxLayer::SandboxLayer()
		: Layer("Sandbox"), m_dockspaceId(0), m_physicsIcon("engine/textures/ui/physicsIcon/LampPhysicsAnim1.png", 30)
	{
		MeshImporter::Initialize();

		g_pEnv->isEditor = true;
		m_iconPlay = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/PlayIcon.png");
		m_iconStop = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/StopIcon.png");
		m_iconSnapToGrid = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/iconSnapToGrid.png");
		m_iconSnapRotation = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/iconSnapRotation.png");
		m_iconSnapScale = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/iconSnapScale.png");
		m_iconShowGizmos = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/iconShowGizmo.png");
		m_iconWorldSpace = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/iconWorldSpace.png");
		m_iconLocalSpace = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/iconLocalSpace.png");
		m_iconSave = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/saveIcon.png");
		m_iconLoad = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/MeshImporter/loadIcon.png");

		//ResourceCache::GetAsset<Texture2D>("assets/textures/TeddyTextures/teddy_albedo.ktx2");

		LevelManager::Get()->Load("assets/levels/testLevel/data.level");

		m_sandboxController = CreateRef<SandboxController>();

		m_windows.emplace_back(new MeshImporterPanel("Mesh Importer"));
		m_windows.emplace_back(new GraphKey("Visual Scripting"));
		m_windows.emplace_back(new MaterialEditorPanel("Material Editor"));
		m_windows.emplace_back(new LayerViewPanel("Layer View", &m_pSelectedObject));
		m_windows.emplace_back(new EnvironmentEditorPanel("Environment Panel"));
		m_windows.emplace_back(new AssetBrowserPanel());
		m_windows.emplace_back(new CreatePanel(&m_pSelectedObject));
		m_windows.emplace_back(new TerrainEditorPanel("Terrain Editor"));
		m_windows.emplace_back(new RenderGraphPanel("Render Graph"));

		Application::Get().GetWindow().Maximize();

		SetupFromConfig();
	}

	SandboxLayer::~SandboxLayer()
	{
		for (auto p : m_windows)
		{
			delete p;
		}
		m_windows.clear();
		
		MeshImporter::Shutdown();
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

			switch (m_sceneState)
			{
				case SceneState::Edit:
				{
					level->UpdateEditor(e.GetTimestep(), m_sandboxController->GetCameraController()->GetCamera());
					break;
				}
				case SceneState::Play:
				{
					level->UpdateRuntime(e.GetTimestep());
					break;
				}
				case SceneState::Simulating:
				{
					level->UpdateSimulation(e.GetTimestep(), m_sandboxController->GetCameraController()->GetCamera());
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

		UpdatePerspective();
		UpdateProperties();
		UpdateLogTool();
		UpdateLevelSettings();
		UpdateRenderingSettings();
		UpdateStatistics();
		//UpdateMainToolbar();

		ImGuiUpdateEvent e;
		OnEvent(e);

		m_uiTotalTime = timerTotal.GetTime();
	}

	void SandboxLayer::OnEvent(Event& e)
	{
		if (m_sceneState == SceneState::Play && m_pGame)
		{
			m_pGame->OnEvent(e);
		}

		for (auto pWindow : m_windows)
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

		RenderCommand::ClearFrame();

		if (LevelManager::GetActive())
		{
			auto level = LevelManager::GetActive();

			switch (m_sceneState)
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

		AppRenderEvent e;
		for (const auto& window : m_windows)
		{
			window->OnEvent(e);
		}
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
					if (m_sceneState == SceneState::Edit)
					{
						OnLevelPlay();
					}
					else if (m_sceneState == SceneState::Play)
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
		m_sceneState = SceneState::Play;
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
		m_sceneState = SceneState::Edit;
		m_pSelectedObject = nullptr;

		m_runtimeLevel->OnRuntimeEnd();

		LevelManager::Get()->SetActive(m_editLevel);
		m_editLevel->SetIsPlaying(false);

		m_runtimeLevel = nullptr;
		m_pGame = nullptr;
	}

	void SandboxLayer::OnSimulationStart()
	{
		m_sceneState = SceneState::Simulating;
		m_pSelectedObject = nullptr;

		m_editLevel = LevelManager::GetActive();
		m_runtimeLevel = CreateRef<Level>(*m_editLevel);

		LevelManager::Get()->SetActive(m_runtimeLevel);
		m_runtimeLevel->SetIsPlaying(true);
		m_runtimeLevel->OnSimulationStart();
	}

	void SandboxLayer::OnSimulationStop()
	{
		m_sceneState = SceneState::Edit;
		m_pSelectedObject = nullptr;

		m_runtimeLevel->OnSimulationEnd();
		LevelManager::Get()->SetActive(m_editLevel);
		m_editLevel->SetIsPlaying(false);

		m_runtimeLevel = nullptr;
	}
}