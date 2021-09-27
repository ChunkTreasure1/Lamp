#include "lppch.h"
#include "Sandbox3D.h"

#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Renderer3D.h"

#include <Lamp/Level/LevelSystem.h>
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

		m_pLevel = LevelSystem::LoadLevel("assets/levels/Level.level");
		ResourceCache::GetAsset<Texture2D>("engine/textures/default/defaultTexture.png");

		//Make sure the sandbox controller is created after level has been loaded
		m_SandboxController = CreateRef<SandboxController>();

		m_pWindows.push_back(new SandboxMeshImporter("Mesh Importer"));
		m_pWindows.push_back(new GraphKey("Visual Scripting"));
		m_pWindows.push_back(new MaterialEditor("Material Editor"));
		m_pWindows.push_back(new RenderGraphPanel("Render Graph"));
		m_pWindows.push_back(new LayerView("Layer View"));

		SetupFromConfig();
		CreateRenderPasses();
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
		m_SelectionBuffer->ClearAttachment(0, 0);

		if (Input::IsMouseButtonPressed(1) && (m_PerspectiveHover || m_RightMousePressed))
		{
			m_SandboxController->Update(e.GetTimestep());
		}

		GetInput();

		m_SelectionBuffer->ClearAttachment(0, -1);

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

		for	(auto& window : m_BufferWindows)
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

		m_SandboxBuffer->Resize(width, height);
		m_GBuffer->Resize(width, height);
		m_SelectionBuffer->Resize(width, height);
		m_SSAOBuffer->Resize(width, height);
		m_SSAOBlurBuffer->Resize(width, height);

		m_SandboxController->GetCameraController()->UpdateProjection(width, height);

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
			shadowBuffer.Width = 8192;
			shadowBuffer.Height = 8192;

			RenderPassSpecification shadowSpec;
			shadowSpec.TargetFramebuffer = Lamp::Framebuffer::Create(shadowBuffer);
			shadowSpec.type = PassType::DirShadow;
			shadowSpec.Name = "DirShadowPass";

			shadowSpec.staticUniforms =
			{
				{ "u_Model", UniformType::Mat4, RenderData::Transform }
			};

			shadowSpec.cullFace = Lamp::CullFace::Front;
			shadowSpec.renderShader = Lamp::ShaderLibrary::GetShader("dirShadow");

			m_DirShadowBuffer = shadowSpec.TargetFramebuffer;

			m_BufferWindows.push_back(BufferWindow(shadowSpec.TargetFramebuffer, "DirShadowBuffer"));

			Ref<RenderPass> shadowPass = CreateRef<RenderPass>(shadowSpec);
			RenderPassManager::Get()->AddPass(shadowPass);
		}
		///////////////////////

		/////////Point shadow pass/////
		{
			RenderPassSpecification shadowSpec;
			shadowSpec.type = PassType::PointShadow;
			shadowSpec.Name = "PointShadowPass";

			Ref<RenderPass> shadowPass = CreateRef<RenderPass>(shadowSpec);
			RenderPassManager::Get()->AddPass(shadowPass);
		}
		/////////////////////////////

		/////Main//////
		{
			FramebufferSpecification mainBuffer;
			mainBuffer.Attachments =
			{
				{ FramebufferTextureFormat::RGBA16F, FramebufferTexureFiltering::Nearest, FramebufferTextureWrap::ClampToEdge }, //position + metallic
				{ FramebufferTextureFormat::RGBA16F, FramebufferTexureFiltering::Nearest, FramebufferTextureWrap::ClampToEdge }, //normal + roughness
				{ FramebufferTextureFormat::RGBA16F, FramebufferTexureFiltering::Nearest, FramebufferTextureWrap::ClampToEdge }, //albedo + ao
			};

			mainBuffer.Attachments.Renderbuffers =
			{
				{ FramebufferRenderbufferType::Depth }
			};

			mainBuffer.ClearColor = m_ClearColor;
			mainBuffer.Width = 1280;
			mainBuffer.Height = 720;
			mainBuffer.Samples = 1;

			RenderPassSpecification passSpec;

			passSpec.TargetFramebuffer = Lamp::Framebuffer::Create(mainBuffer);
			passSpec.Name = "MainPass";
			passSpec.type = PassType::Geometry;

			m_GBuffer = passSpec.TargetFramebuffer;

			passSpec.staticUniforms =
			{
				{ "u_Model", UniformType::Mat4, RenderData::Transform },
				{ "u_Material.albedo", UniformType::Int, 0 },
				{ "u_Material.normal", UniformType::Int, 1},
				{ "u_Material.mro", UniformType::Int, 2}
			};

			passSpec.clearType = Lamp::ClearType::ColorDepth;
			passSpec.cullFace = Lamp::CullFace::Back;
			passSpec.renderShader = Lamp::ShaderLibrary::GetShader("gbuffer");

			Ref<RenderPass> renderPass = CreateRef<RenderPass>(passSpec);
			RenderPassManager::Get()->AddPass(renderPass);
		}
		///////////////

		/////SSAOMain/////
		{
			FramebufferSpecification mainBuffer;
			mainBuffer.Attachments =
			{
				{ FramebufferTextureFormat::RED, FramebufferTexureFiltering::Nearest, FramebufferTextureWrap::ClampToEdge }
			};

			mainBuffer.ClearColor = m_ClearColor;
			mainBuffer.Width = 1280;
			mainBuffer.Height = 720;
			mainBuffer.Samples = 1;

			RenderPassSpecification passSpec;
			passSpec.TargetFramebuffer = Lamp::Framebuffer::Create(mainBuffer);
			m_SSAOBuffer = passSpec.TargetFramebuffer;

			passSpec.type = PassType::SSAO;
			passSpec.Name = "SSAOMain";

			passSpec.staticUniforms =
			{
				{ "u_GBuffer.position", UniformType::Int, 0 },
				{ "u_GBuffer.normal", UniformType::Int, 1 },
				{ "u_Noise", UniformType::Int, 2 }
			};

			passSpec.framebuffers =
			{
				{ m_GBuffer, TextureType::Color, 0, 0 },
				{ m_GBuffer, TextureType::Color, 1, 1 }
			};

			passSpec.textures =
			{
				{ Renderer3D::GetSettings().SSAONoiseTexture, 2 }
			};

			passSpec.clearType = Lamp::ClearType::ColorDepth;
			passSpec.cullFace = Lamp::CullFace::Back;
			passSpec.drawType = Lamp::DrawType::Quad;
			passSpec.renderShader = Lamp::ShaderLibrary::GetShader("SSAOMain");


			Ref<RenderPass> ssaoPath = CreateRef<RenderPass>(passSpec);
			RenderPassManager::Get()->AddPass(ssaoPath);
		} 
		//////////////////

		/////SSAOBlur/////
		{
			FramebufferSpecification mainBuffer;
			mainBuffer.Attachments =
			{
				{ FramebufferTextureFormat::RED, FramebufferTexureFiltering::Nearest, FramebufferTextureWrap::ClampToEdge }
			};

			mainBuffer.ClearColor = m_ClearColor;
			mainBuffer.Width = 1280;
			mainBuffer.Height = 720;
			mainBuffer.Samples = 1;

			RenderPassSpecification passSpec;
			passSpec.TargetFramebuffer = Lamp::Framebuffer::Create(mainBuffer);
			passSpec.type = PassType::SSAOBlur;
			passSpec.Name = "SSAOBlur";

			passSpec.staticUniforms =
			{
				{ "u_SSAO", UniformType::Int, 0 }
			};

			passSpec.framebuffers =
			{
				{ m_SSAOBuffer, TextureType::Color, 0, 0 }
			};

			passSpec.clearType = Lamp::ClearType::Color;
			passSpec.cullFace = Lamp::CullFace::Back;
			passSpec.drawType = Lamp::DrawType::Quad;
			passSpec.renderShader = Lamp::ShaderLibrary::GetShader("SSAOBlur");

			m_SSAOBlurBuffer = passSpec.TargetFramebuffer;

			Ref<RenderPass> ssaoPath = CreateRef<RenderPass>(passSpec);
			RenderPassManager::Get()->AddPass(ssaoPath);
		}
		//////////////////

		/////Lightning/////
		{
			FramebufferSpecification lightBuffer;
			lightBuffer.Attachments =
			{
				{ FramebufferTextureFormat::RGBA8, FramebufferTexureFiltering::Nearest, FramebufferTextureWrap::ClampToEdge }
			};

			lightBuffer.Attachments.Renderbuffers =
			{
				{ FramebufferRenderbufferType::Depth }
			};

			lightBuffer.ClearColor = m_ClearColor;
			lightBuffer.Width = 1280;
			lightBuffer.Height = 720;
			lightBuffer.Samples = 1;

			RenderPassSpecification passSpec;
			passSpec.TargetFramebuffer = Lamp::Framebuffer::Create(lightBuffer);
			m_SandboxBuffer = passSpec.TargetFramebuffer;

			passSpec.Name = "LightPass";
			passSpec.type = PassType::Lightning;

			passSpec.staticUniforms =
			{
				{ "u_GBuffer.position", UniformType::Int, 0 },
				{ "u_GBuffer.normal", UniformType::Int, 1 },
				{ "u_GBuffer.albedo", UniformType::Int, 2 },
				{ "u_ShadowMap", UniformType::Int, 3 },
				{ "u_IrradianceMap", UniformType::Int, 4 },
				{ "u_PrefilterMap", UniformType::Int, 5 },
				{ "u_BRDFLUT", UniformType::Int, 6 },
				{ "u_SSAO", UniformType::Int, 7 }
			};

			passSpec.dynamicUniforms =
			{
				{ "u_Exposure", UniformType::Float, RegisterData(&Renderer3D::GetSettings().HDRExposure) },
				{ "u_Gamma", UniformType::Float, RegisterData(&Renderer3D::GetSettings().Gamma) },
				{ "u_DirectionalLight.direction", UniformType::Float3, RegisterData(&g_pEnv->DirLight.Position)},
				{ "u_DirectionalLight.color", UniformType::Float3, RegisterData(&g_pEnv->DirLight.Color) },
				{ "u_DirectionalLight.intensity", UniformType::Float, RegisterData(&g_pEnv->DirLight.Intensity) },
			};

			passSpec.framebuffers =
			{
				{ m_GBuffer, TextureType::Color, 0, 0 },
				{ m_GBuffer, TextureType::Color, 1, 1 },
				{ m_GBuffer, TextureType::Color, 2, 2 },
				{ m_DirShadowBuffer, TextureType::Depth, 3, 0 },
				{ g_pEnv->pSkyboxBuffer, TextureType::Color, 4, 0 },
				{ g_pEnv->pSkyboxBuffer, TextureType::Color, 5, 1 },
				{ g_pEnv->pSkyboxBuffer, TextureType::Color, 6, 2 },
				{ m_SSAOBlurBuffer, TextureType::Color, 7, 0 }
			};

			passSpec.framebufferCommands =
			{
				{ m_SandboxBuffer, m_GBuffer, FramebufferCommand::Copy }
			};

			passSpec.clearType = Lamp::ClearType::ColorDepth;
			passSpec.cullFace = Lamp::CullFace::Back;
			passSpec.drawType = Lamp::DrawType::Quad;
			passSpec.renderShader = Lamp::ShaderLibrary::GetShader("deferred");

			Ref<RenderPass> renderPass = CreateRef<RenderPass>(passSpec);
			RenderPassManager::Get()->AddPass(renderPass);
		}
		///////////////////

		/////Forward Rendering/////
		{
			std::vector<std::function<void()>> ptrs;
			ptrs.push_back(LP_EXTRA_RENDER(Sandbox3D::RenderGrid));
			ptrs.push_back(LP_EXTRA_RENDER(Sandbox3D::RenderSkybox));

			RenderPassSpecification passSpec;
			passSpec.ExtraRenders = ptrs;

			passSpec.TargetFramebuffer = m_SandboxBuffer;
			passSpec.Name = "ForwardPass";
			passSpec.type = PassType::Forward;

			passSpec.clearType = Lamp::ClearType::ColorDepth;
			passSpec.cullFace = Lamp::CullFace::Back;
			passSpec.drawType = Lamp::DrawType::All;

			Ref<RenderPass> renderPass = CreateRef<RenderPass>(passSpec);
			RenderPassManager::Get()->AddPass(renderPass);
		}
		///////////////////////////

		/////Selection/////
		{
			FramebufferSpecification spec;
			spec.Attachments =
			{
				{ FramebufferTextureFormat::RED_INTEGER, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::Repeat },
				{ FramebufferTextureFormat::DEPTH32F, FramebufferTexureFiltering::Linear, FramebufferTextureWrap::Repeat }
			};
			spec.Width = 1280;
			spec.Height = 720;
			spec.Samples = 1;

			RenderPassSpecification passSpec;
			passSpec.TargetFramebuffer = Lamp::Framebuffer::Create(spec);
			m_SelectionBuffer = passSpec.TargetFramebuffer;
			passSpec.Name = "SelectionPass";
			passSpec.type = PassType::Selection;

			passSpec.staticUniforms =
			{
				{ "u_ObjectId", UniformType::Int, RenderData::ID },
				{ "u_Model", UniformType::Mat4, RenderData::Transform }
			};

			passSpec.renderShader = Lamp::ShaderLibrary::GetShader("selection");

			Ref<RenderPass> pass = CreateRef<RenderPass>(passSpec);
			RenderPassManager::Get()->AddPass(pass);
		}
		///////////////////
	}
}