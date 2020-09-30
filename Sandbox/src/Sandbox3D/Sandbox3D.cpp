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

#include <Lamp/Rendering/RenderCommand.h>
#include <Lamp/Rendering/RenderPass.h>

namespace Sandbox3D
{
	Sandbox3D::Sandbox3D()
		: Lamp::Layer("Sandbox3D"), m_SelectedFile(""), m_DockspaceID(0), m_pShader(nullptr)
	{
		m_SandboxController = CreateRef<SandboxController>();
		m_pGame = CreateScope<Game>();	
		m_pGame->OnStart();

		g_pEnv->ShouldRenderBB = true;
		CreateRenderPasses();
	}

	bool Sandbox3D::OnUpdate(Lamp::AppUpdateEvent& e)
	{
		m_SandboxController->Update(e.GetTimestep());
		GetInput();

		Lamp::RenderPassManager::Get()->RenderPasses();

		return true;
	}

	void Sandbox3D::OnImGuiRender(Lamp::Timestep ts)
	{
		CreateDockspace();

		UpdateProperties();
		UpdatePerspective();
		UpdateAssetBrowser();
		UpdateModelImporter();
		UpdateLayerView();
		UpdateAddComponent();
		UpdateCreateTool();
		UpdateLogTool();
	}

	void Sandbox3D::OnEvent(Lamp::Event& e)
	{
		m_pGame->OnEvent(e);
		m_SandboxController->OnEvent(e);

		Lamp::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Lamp::MouseMovedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnMouseMoved));
		dispatcher.Dispatch<Lamp::AppUpdateEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnUpdate));
		dispatcher.Dispatch<Lamp::AppItemClickedEvent>(LP_BIND_EVENT_FN(Sandbox3D::OnItemClicked));
	}

	bool Sandbox3D::OnItemClicked(Lamp::AppItemClickedEvent& e)
	{
		m_SelectedFile = e.GetFile();
		return true;
	}

	void Sandbox3D::GetInput()
	{
		if (Lamp::Input::IsMouseButtonPressed(0))
		{
			m_MousePressed = true;
		}
		else if (Lamp::Input::IsMouseButtonReleased(0))
		{
			m_MousePressed = false;
		}

		if (Lamp::Input::IsKeyPressed(LP_KEY_1))
		{
			m_ImGuizmoOperation = ImGuizmo::TRANSLATE;
		}

		if (Lamp::Input::IsKeyPressed(LP_KEY_2))
		{
			m_ImGuizmoOperation = ImGuizmo::ROTATE;
		}

		if (Lamp::Input::IsKeyPressed(LP_KEY_3))
		{
			m_ImGuizmoOperation = ImGuizmo::SCALE;
		}

		if (Lamp::Input::IsKeyPressed(LP_KEY_DELETE))
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
		Lamp::Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, 0.f), glm::vec3(5.f, 0.f, 0.f), 1.f);
		Lamp::Renderer3D::DrawLine(glm::vec3(0.f, 0.f, -5.f), glm::vec3(0.f, 0.f, 5.f), 1.f);

		for (size_t x = 1; x <= 10; x++)
		{
			Lamp::Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, 0.5f * (float)x), glm::vec3(5.f, 0.f, 0.5f * (float)x), 1.f);
		}

		for (size_t x = 1; x <= 10; x++)
		{
			Lamp::Renderer3D::DrawLine(glm::vec3(-5.f, 0.f, -0.5f * (float)x), glm::vec3(5.f, 0.f, -0.5f * (float)x), 1.f);
		}

		for (size_t z = 1; z <= 10; z++)
		{
			Lamp::Renderer3D::DrawLine(glm::vec3(0.5f * (float)z, 0.f, -5.f), glm::vec3(0.5f * (float)z, 0.f, 5.f), 1.f);
		}

		for (size_t z = 1; z <= 10; z++)
		{
			Lamp::Renderer3D::DrawLine(glm::vec3(-0.5f * (float)z, 0.f, -5.f), glm::vec3(-0.5f * (float)z, 0.f, 5.f), 1.f);
		}
	}

	void Sandbox3D::CreateRenderPasses()
	{
		glm::mat4 proj = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.1f, 100.f);
		glm::mat4 view = glm::lookAt(g_pEnv->DirLightInfo.Position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
		glm::mat4 lightViewProj = proj * view;

		{
			Lamp::RenderPassInfo passInfo;
			passInfo.Camera = m_SandboxController->GetCameraController()->GetCamera();
			passInfo.IsShadowPass = true;
			passInfo.ViewProjection = lightViewProj;
			passInfo.ClearColor = m_ClearColor;

			Ref<Lamp::RenderPass> renderPass = CreateRef<Lamp::RenderPass>(Lamp::Renderer3D::GetShadowBuffer(), passInfo);

			Lamp::RenderPassManager::Get()->AddPass(renderPass);
		}

		{
			//Creating the render pass info
			Lamp::RenderPassInfo passInfo;
			passInfo.Camera = m_SandboxController->GetCameraController()->GetCamera();
			passInfo.IsShadowPass = false;
			passInfo.ViewProjection = m_SandboxController->GetCameraController()->GetCamera()->GetViewProjectionMatrix();
			passInfo.LightViewProjection = lightViewProj;
			passInfo.ClearColor = m_ClearColor;

			Ref<Lamp::RenderPass> renderPass = CreateRef<Lamp::RenderPass>(Lamp::Renderer3D::GetFrameBuffer(), passInfo);
			
			Lamp::RenderPassManager::Get()->AddPass(renderPass);
		}
	}
}