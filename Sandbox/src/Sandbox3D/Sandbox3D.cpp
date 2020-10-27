#include "lppch.h"
#include "Sandbox3D.h"

#include "Lamp/Rendering/Renderer2D.h"
#include "Lamp/Rendering/Renderer3D.h"
#include <Lamp/Physics/Collision.h>
#include <Lamp/Objects/Brushes/BrushManager.h>
#include <Lamp/Level/LevelSystem.h>
#include <Lamp/Event/ApplicationEvent.h>

#include <Lamp/Physics/Colliders/BoundingSphere.h>
#include <Lamp/Physics/Colliders/AABB.h>
#include <Lamp/Physics/PhysicsEngine.h>
#include <Lamp/Physics/Physics.h>
#include <Lamp/Objects/Entity/BaseComponents/MeshComponent.h>
#include <Lamp/Meshes/GeometrySystem.h>

#include <Lamp/Objects/ObjectLayer.h>
#include <Lamp/Objects/Entity/BaseComponents/LightComponent.h>
#include <Lamp/Core/Game.h>

#include <Lamp/Rendering/RenderPass.h>

namespace Sandbox3D
{
	using namespace Lamp;

	Sandbox3D::Sandbox3D()
		: Layer("Sandbox3D"), m_SelectedFile(""), m_DockspaceID(0), m_pShader(nullptr), m_PerspecticeCommands(100)
	{
		m_pGame = CreateScope<Game>();
		m_pGame->OnStart();

		//Make sure the sandbox controller is created after level has been loaded
		m_SandboxController = CreateRef<SandboxController>();
		g_pEnv->ShouldRenderBB = true;

		SetupFromConfig();
		CreateRenderPasses();
	}

	bool Sandbox3D::OnUpdate(AppUpdateEvent& e)
	{
		m_SandboxController->Update(e.GetTimestep());
		GetInput();

		RenderPassManager::Get()->RenderPasses();

		return true;
	}

	void Sandbox3D::OnImGuiRender(Timestep ts)
	{
		CreateDockspace();

		UpdateProperties();
		UpdatePerspective();
		UpdateAssetBrowser();
		UpdateModelImporter();
		UpdateLayerView();
		UpdateCreateTool();
		UpdateLogTool();
		UpdateLevelSettings();
	}

	void Sandbox3D::OnEvent(Event& e)
	{
		m_pGame->OnEvent(e);
		m_SandboxController->OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseMovedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnMouseMoved));
		dispatcher.Dispatch<AppUpdateEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnUpdate));
		dispatcher.Dispatch<AppItemClickedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnItemClicked));
		dispatcher.Dispatch<WindowCloseEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnWindowClose));
		dispatcher.Dispatch<KeyPressedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnKeyPressed));
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
		}

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

	void Sandbox3D::CreateRenderPasses()
	{
		RenderPassInfo passInfo;
		passInfo.Camera = m_SandboxController->GetCameraController()->GetCamera();
		passInfo.IsShadowPass = true;
		passInfo.DirLight = g_pEnv->DirLight;
		passInfo.ClearColor = m_ClearColor;

		Ref<RenderPass> shadowPass = CreateRef<RenderPass>(Renderer3D::GetShadowBuffer(), passInfo);
		RenderPassManager::Get()->AddPass(shadowPass);

		passInfo.IsShadowPass = false;

		std::vector<std::function<void()>> ptrs;

		ptrs.push_back(LP_EXTRA_RENDER(Sandbox3D::RenderGrid));
		ptrs.push_back(LP_EXTRA_RENDER(Sandbox3D::RenderSkybox));

		Ref<RenderPass> renderPass = CreateRef<RenderPass>(Renderer3D::GetFrameBuffer(), passInfo, ptrs);
		RenderPassManager::Get()->AddPass(renderPass);
	}
}