#include "lppch.h"
#include "Sandbox3D.h"

#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Renderer3D.h"

#include <Lamp/Event/ApplicationEvent.h>

#include <Lamp/Core/Game.h>

#include <Lamp/Rendering/RenderPass.h>

#include "Windows/SandboxMeshImporter.h"
#include "Windows/GraphKey.h"
#include "Windows/MaterialEditor.h"
#include "Windows/RenderGraphPanel.h"
#include "Windows/LayerView.h"

#include <Lamp/Rendering/Shadows/PointShadowBuffer.h>

#include <Platform/OpenGL/OpenGLFramebuffer.h>
#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>

namespace Sandbox3D
{
	using namespace Lamp;

	Sandbox3D::Sandbox3D()
		: Layer("Sandbox3D"), m_DockspaceID(0), m_PhysicsIcon("engine/textures/ui/physicsIcon/LampPhysicsAnim1.png", 30)
	{
		g_pEnv->IsEditor = true;
		m_IconPlay = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/PlayIcon.png");
		m_IconStop = ResourceCache::GetAsset<Texture2D>("engine/textures/ui/StopIcon.png");

		m_pLevel = ResourceCache::GetAsset<Level>("assets/testLevel.level");
		ResourceCache::GetAsset<Texture2D>("engine/textures/default/defaultTexture.png");

		//Make sure the sandbox controller is created after level has been loaded
		m_SandboxController = CreateRef<SandboxController>();
		Renderer3D::GetSettings().RenderGraph = ResourceCache::GetAsset<RenderGraph>("assets/testGraph.rendergraph");
		Renderer3D::GetSettings().RenderGraph->Start();

		m_pWindows.push_back(new SandboxMeshImporter("Mesh Importer"));
		m_pWindows.push_back(new GraphKey("Visual Scripting"));
		m_pWindows.push_back(new MaterialEditor("Material Editor"));
		m_pWindows.push_back(new RenderGraphPanel("Render Graph"));
		m_pWindows.push_back(new LayerView("Layer View"));

		SetupFromConfig();
	}

	Sandbox3D::~Sandbox3D()
	{
		for (auto p : m_pWindows)
		{
			delete p;
		}

		m_BufferWindows.clear();
		m_pWindows.clear();
	}

	bool Sandbox3D::OnUpdate(AppUpdateEvent& e)
	{
		LP_PROFILE_FUNCTION();

		if (Input::IsMouseButtonPressed(1) && (m_PerspectiveHover || m_RightMousePressed))
		{
			m_SandboxController->Update(e.GetTimestep());
		}

		GetInput();

		{
			LP_PROFILE_SCOPE("Sandbox3D::Update::LevelUpdate")
				switch (m_SceneState)
				{
					case SceneState::Edit:
					{
						g_pEnv->pLevel->UpdateEditor(e.GetTimestep(), m_SandboxController->GetCameraController()->GetCamera());
						break;
					}
					case SceneState::Play:
					{
						g_pEnv->pLevel->UpdateRuntime(e.GetTimestep());
						break;
					}
					case SceneState::Simulating:
					{
						g_pEnv->pLevel->UpdateSimulation(e.GetTimestep(), m_SandboxController->GetCameraController()->GetCamera());
					}

					default:
						break;
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
					for (auto pFunc : pWindow->GetRenderFuncs())
					{
						pFunc();
					}
				}
			}
		}

		return false;
	}

	void Sandbox3D::OnImGuiRender(Timestep ts)
	{
		LP_PROFILE_FUNCTION();
		CreateDockspace();

		UpdateProperties();
		UpdatePerspective();
		UpdateCreateTool();
		UpdateLogTool();
		UpdateLevelSettings();
		UpdateRenderingSettings();
		UpdateRenderPassView();
		UpdateShaderView();
		UpdateToolbar();
		m_assetManager.OnImGuiRender();

		for (auto& window : m_BufferWindows)
		{
			window.Update();
		}

		ImGuiUpdateEvent e;
		OnEvent(e);
	}

	void Sandbox3D::OnEvent(Event& e)
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

		if (Input::IsMouseButtonReleased(1))
		{
			m_SandboxController->GetCameraController()->SetHasControl(false);
			m_RightMousePressed = false;

			Application::Get().GetWindow().ShowCursor(true);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}

		if (Input::IsMouseButtonPressed(1) && (m_PerspectiveHover || m_RightMousePressed))
		{
			m_SandboxController->OnEvent(e);
			m_RightMousePressed = true;

			Application::Get().GetWindow().ShowCursor(false);
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseMovedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnMouseMoved));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnUpdate));
		dispatcher.Dispatch<WindowCloseEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnKeyPressed));
		dispatcher.Dispatch<ImGuiBeginEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnImGuiBegin));
		dispatcher.Dispatch<EditorViewportSizeChangedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnViewportSizeChanged));
	}

	bool Sandbox3D::OnKeyPressed(KeyPressedEvent& e)
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

					if (g_pEnv->pLevel->Path.empty())
					{
						SaveLevelAs();
						break;
					}
					else
					{
						g_pEnv->pAssetManager->SaveAsset(g_pEnv->pLevel);
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

	bool Sandbox3D::OnImGuiBegin(ImGuiBeginEvent& e)
	{
		ImGuizmo::BeginFrame();
		return true;
	}

	bool Sandbox3D::OnViewportSizeChanged(Lamp::EditorViewportSizeChangedEvent& e)
	{
		uint32_t width = e.GetWidth();
		uint32_t height = e.GetHeight();

		for (const auto& buffer : Renderer3D::GetSettings().UseViewportSize)
		{
			buffer->Resize(width, height);
		}

		m_SandboxController->GetCameraController()->UpdateProjection(width, height);
		Renderer3D::GetSettings().BufferSize = { width, height };

		return false;
	}

	void Sandbox3D::GetInput()
	{
		if (Input::IsMouseButtonPressed(0))
		{
			m_MousePressed = true;
		}
		else if (Input::IsMouseButtonReleased(0))
		{
			m_MousePressed = false;
		}

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

	void Sandbox3D::RenderSkybox()
	{
		Renderer3D::DrawSkybox();
	}

	void Sandbox3D::RenderLines()
	{
		for (auto& p : m_Lines)
		{
		}
	}

	void Sandbox3D::OnLevelPlay()
	{
		m_SceneState = SceneState::Play;
		m_pSelectedObject = nullptr;
		m_pRuntimeLevel = CreateRef<Level>(*m_pLevel);

		g_pEnv->pLevel = m_pRuntimeLevel;
		m_pRuntimeLevel->SetIsPlaying(true);
		m_pRuntimeLevel->OnRuntimeStart();

		m_pGame = CreateScope<Game>();
		m_pGame->OnStart();
	}

	void Sandbox3D::OnLevelStop()
	{
		m_SceneState = SceneState::Edit;
		m_pSelectedObject = nullptr;

		m_pRuntimeLevel->OnRuntimeEnd();
		g_pEnv->pLevel = m_pLevel;

		m_pRuntimeLevel = nullptr;
		m_pGame = nullptr;
	}

	void Sandbox3D::OnSimulationStart()
	{
		m_SceneState = SceneState::Simulating;
		m_pSelectedObject = nullptr;

		m_pRuntimeLevel = CreateRef<Level>(*m_pLevel);
		m_pRuntimeLevel->SetIsPlaying(true);
		g_pEnv->pLevel = m_pRuntimeLevel;
		m_pRuntimeLevel->OnSimulationStart();
	}

	void Sandbox3D::OnSimulationStop()
	{
		m_SceneState = SceneState::Edit;
		m_pSelectedObject = nullptr;

		m_pRuntimeLevel->OnSimulationEnd();
		g_pEnv->pLevel = m_pLevel;

		m_pRuntimeLevel = nullptr;
	}
}