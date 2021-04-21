#include "lppch.h"
#include "Sandbox3D.h"

#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Renderer3D.h"

#include <Lamp/Level/LevelSystem.h>
#include <Lamp/Event/ApplicationEvent.h>

#include <Lamp/Physics/PhysicsEngine.h>

#include <Lamp/Objects/ObjectLayer.h>
#include <Lamp/Core/Game.h>

#include <Lamp/Rendering/RenderPass.h>

#include "Windows/ModelImporter.h"
#include "Windows/GraphKey.h"
#include "Windows/MaterialEditor.h"

#include <Lamp/Rendering/Shadows/PointShadowBuffer.h>
#include <Lamp/AssetSystem/AssetManager.h>

#include <Platform/OpenGL/OpenGLFramebuffer.h>
#include <imnodes.h>
namespace Sandbox3D
{
	using namespace Lamp;

	Sandbox3D::Sandbox3D()
		: Layer("Sandbox3D"), m_SelectedFile(""), m_DockspaceID(0), m_pShader(nullptr)
	{
		m_pGame = CreateScope<Game>();
		m_pGame->OnStart();

		auto level = LevelSystem::LoadLevel("assets/levels/Level.level");
		g_pEnv->pAssetManager->LoadTexture("engine/textures/default/defaultTexture.png", nullptr);

		//Make sure the sandbox controller is created after level has been loaded
		m_SandboxController = CreateRef<SandboxController>();

		m_pWindows.push_back(new ModelImporter("Model Importer"));
		m_pWindows.push_back(new GraphKey("Visual Scripting"));
		m_pWindows.push_back(new MaterialEditor("Material Editor"));

		FramebufferSpecification spec;
		spec.Attachments =
		{
			{ FramebufferTextureFormat::RGBA8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge }
		};

		spec.Height = 720;
		spec.Width = 1280;
		spec.ClearColor = { 0.1f, 0.1f, 0.1f, 1.f };

		m_SecondaryBuffer = Lamp::Framebuffer::Create(spec);
		SetupFromConfig();
		CreateRenderPasses();

		imnodes::Initialize();
	}

	Sandbox3D::~Sandbox3D()
	{
		for (auto p : m_pWindows)
		{
			delete p;
		}

		m_BufferWindows.clear();
		m_pWindows.clear();
		imnodes::Shutdown();

		delete m_pLevel;
	}

	bool Sandbox3D::OnUpdate(AppUpdateEvent& e)
	{
		LP_PROFILE_FUNCTION();
		if (m_IsPlaying != m_ShouldPlay)
		{
			m_IsPlaying = m_ShouldPlay;
		
			if (m_IsPlaying)
			{
				m_pGame->OnStart();
				for (auto& node : NodeRegistry::s_StartNodes())
				{
					node->ActivateOutput(0);
				}
			}
		}

		m_SelectionBuffer->ClearAttachment(0, 0);

		if (m_SandboxController->GetCameraController()->GetRightPressed())
		{
			m_SandboxController->Update(e.GetTimestep());
		}
		else
		{
			m_SandboxController->Update(e.GetTimestep());
		}

		GetInput();

		m_SelectionBuffer->ClearAttachment(0, -1);

		{
			LP_PROFILE_SCOPE("Sandbox3D::Update::Rendering");
			RenderPassManager::Get()->RenderPasses();
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

		glm::vec2 srcSize = { m_SandboxBuffer->GetSpecification().Width, m_SandboxBuffer->GetSpecification().Height };
		m_SecondaryBuffer->Copy(m_SandboxBuffer->GetRendererID(), srcSize);

		return false;
	}

	void Sandbox3D::OnImGuiRender(Timestep ts)
	{
		CreateDockspace();

		UpdateProperties();
		UpdatePerspective();
		UpdateAssetBrowser();
		UpdateLayerView();
		UpdateCreateTool();
		UpdateLogTool();
		UpdateLevelSettings();

		for	(auto& window : m_BufferWindows)
		{
			window.Update();
		}

		ImGuiUpdateEvent e;
		OnEvent(e);
	}

	void Sandbox3D::OnEvent(Event& e)
	{
		if (m_IsPlaying)
		{
			m_pGame->OnEvent(e);
			g_pEnv->pObjectLayerManager->OnEvent(e);
		}

		for (auto pWindow : m_pWindows)
		{
			if (pWindow->GetIsOpen())
			{
				pWindow->OnEvent(e);
			}
		}

		if (m_SandboxController->GetCameraController()->GetRightPressed())
		{
			m_SandboxController->OnEvent(e);
		}
		else
		{
			m_SandboxController->OnEvent(e);
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseMovedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnMouseMoved));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnUpdate));
		dispatcher.Dispatch<AppItemClickedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnItemClicked));
		dispatcher.Dispatch<WindowCloseEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnKeyPressed));
		dispatcher.Dispatch<ImGuiBeginEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnImGuiBegin));
	}

	bool Sandbox3D::OnItemClicked(AppItemClickedEvent& e)
	{
		m_SelectedFile = e.GetFile();
		return true;
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

					if (LevelSystem::GetCurrentLevel()->GetPath().empty())
					{
						SaveLevelAs();
						break;
					}
					else
					{
						LevelSystem::SaveLevel(LevelSystem::GetCurrentLevel());
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
		}

		return false;
	}

	bool Sandbox3D::OnImGuiBegin(ImGuiBeginEvent& e)
	{
		ImGuizmo::BeginFrame();
		return true;
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

	void Sandbox3D::RenderGrid()
	{
		Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, 0.f), glm::vec3(5.f, 0.f, 0.f), 1.f);
		Renderer3D::DrawLine(glm::vec3(0.f, 0.f, -5.f), glm::vec3(0.f, 0.f, 5.f), 1.f);

		for (size_t x = 1; x <= 10; x++)
		{
			Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, 0.5f * (float)x), glm::vec3(5.f, 0.f, 0.5f * (float)x), 1.f);
		}

		for (size_t x = 1; x <= 10; x++)
		{
			Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, -0.5f * (float)x), glm::vec3(5.f, 0.f, -0.5f * (float)x), 1.f);
		}

		for (size_t z = 1; z <= 10; z++)
		{
			Renderer3D::DrawLine(glm::vec3(0.5f * (float)z, 0.f, -5.f), glm::vec3(0.5f * (float)z, 0.f, 5.f), 1.f);
		}

		for (size_t z = 1; z <= 10; z++)
		{
			Renderer3D::DrawLine(glm::vec3(-0.5f * (float)z, 0.f, -5.f), glm::vec3(-0.5f * (float)z, 0.f, 5.f), 1.f);
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
			Renderer3D::DrawLine(p.first, p.second, 1.f);
		}
	}

	void Sandbox3D::CreateRenderPasses()
	{
		/////Shadow pass/////
		{
			FramebufferSpecification shadowBuffer;
			shadowBuffer.Attachments =
			{
				{ FramebufferTextureFormat::DEPTH32F, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToBorder }
			};
			shadowBuffer.ClearColor = m_ClearColor;
			shadowBuffer.Height = 4096;
			shadowBuffer.Width = 4096;

			RenderPassSpecification shadowSpec;
			shadowSpec.TargetFramebuffer = CreateRef<Lamp::OpenGLFramebuffer>(shadowBuffer);
			shadowSpec.Camera = m_SandboxController->GetCameraController()->GetCamera();
			shadowSpec.IsShadowPass = true;
			shadowSpec.Name = "DirShadowPass";

			m_BufferWindows.push_back(BufferWindow(shadowSpec.TargetFramebuffer, "DirShadowBuffer"));

			Ref<RenderPass> shadowPass = CreateRef<RenderPass>(shadowSpec);
			RenderPassManager::Get()->AddPass(shadowPass);
		}
		///////////////////////

		/////////Point shadow pass/////
		{
			RenderPassSpecification shadowSpec;
			shadowSpec.Camera = m_SandboxController->GetCameraController()->GetCamera();
			shadowSpec.IsPointShadowPass = true;
			shadowSpec.Name = "PointShadowPass";

			Ref<RenderPass> shadowPass = CreateRef<RenderPass>(shadowSpec);
			RenderPassManager::Get()->AddPass(shadowPass);
		}
		/////////////////////////////

		/////Selection/////
		{
			FramebufferSpecification spec;
			spec.Attachments =
			{
				{ FramebufferTextureFormat::RED_INTEGER, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::Repeat }
			};
			spec.Height = 1280;
			spec.Width = 720;
			spec.Samples = 1;

			RenderPassSpecification passSpec;
			passSpec.Camera = m_SandboxController->GetCameraController()->GetCamera();
			passSpec.TargetFramebuffer = Lamp::Framebuffer::Create(spec);
			m_SelectionBuffer = passSpec.TargetFramebuffer;
			passSpec.Name = "SelectionPass";

			m_BufferWindows.push_back(BufferWindow(passSpec.TargetFramebuffer, "SelectionPass"));

			Ref<RenderPass> pass = CreateRef<RenderPass>(passSpec);
			RenderPassManager::Get()->AddPass(pass);
		}
		///////////////////

		/////Main//////
		{
			FramebufferSpecification mainBuffer;
			mainBuffer.Attachments =
			{
				{ FramebufferTextureFormat::RGBA8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge, { 1.f, 1.f, 1.f, 1.f }, true },
				{ FramebufferTextureFormat::DEPTH24STENCIL8, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::ClampToEdge }
			};
			mainBuffer.ClearColor = m_ClearColor;
			mainBuffer.Height = 1280;
			mainBuffer.Width = 720;
			mainBuffer.Samples = 2;

			std::vector<std::function<void()>> ptrs;
			ptrs.push_back(LP_EXTRA_RENDER(Sandbox3D::RenderGrid));
			ptrs.push_back(LP_EXTRA_RENDER(Sandbox3D::RenderSkybox));

			RenderPassSpecification passSpec;
			passSpec.Camera = m_SandboxController->GetCameraController()->GetCamera();
			passSpec.ExtraRenders = ptrs;

			passSpec.TargetFramebuffer = Lamp::Framebuffer::Create(mainBuffer);
			passSpec.Name = "MainPass";

			m_SandboxBuffer = passSpec.TargetFramebuffer;

			Ref<RenderPass> renderPass = CreateRef<RenderPass>(passSpec);
			RenderPassManager::Get()->AddPass(renderPass);
		}
		///////////////
	}
}