#pragma once

#include <Lamp.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <glm/gtc/type_ptr.hpp>
#include <Lamp/Rendering/Cameras/PerspectiveCameraController.h>
#include <ImGuizmo/ImGuizmo.h>
#include <Lamp/Objects/Brushes/Brush.h>
#include <Lamp/Event/MouseEvent.h>
#include "SandboxController.h"

#include <Game/Game.h>

namespace Sandbox3D
{
	class Sandbox3D : public Lamp::Layer
	{
	public:
		Sandbox3D();

		bool OnUpdate(Lamp::AppUpdateEvent& e);

		virtual void OnImGuiRender(Lamp::Timestep ts) override;
		virtual void OnEvent(Lamp::Event& e) override;

	private:
		void CreateDockspace();
		void GetInput();
		void RenderGrid();
		void RenderSkybox();
		void CreateRenderPasses();
		void SetupFromConfig();

		bool OnMouseMoved(Lamp::MouseMovedEvent& e);
		bool OnItemClicked(Lamp::AppItemClickedEvent& e);
		bool OnWindowClose(Lamp::WindowCloseEvent& e);

		//ImGui
		void UpdatePerspective();
		void UpdateAssetBrowser();
		void UpdateProperties();
		void UpdateModelImporter();
		void UpdateLayerView();
		void UpdateAddComponent();
		void UpdateCreateTool();
		void UpdateLogTool();

	private:
		Scope<Game> m_pGame;
		Ref<SandboxController> m_SandboxController;
		//---------------Editor-----------------
		glm::vec3 m_FColor = glm::vec3{ 0.1f, 0.1f, 0.1f };
		glm::vec4 m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
		glm::vec2 m_PerspectiveSize = glm::vec2(0.f);
		ImGuiID m_DockspaceID;

		//Perspective
		bool m_PerspectiveOpen = true;
		const float m_AspectRatio = 1.7f;
		bool m_PerspectiveFocused = false;
		ImGuizmo::OPERATION m_ImGuizmoOperation = ImGuizmo::TRANSLATE;
		
		//Asset browser
		Lamp::File m_SelectedFile;
		int m_CurrSample = -1;
		bool m_AssetBrowserOpen = true;

		//Inspector
		bool m_MousePressed = false;
		bool m_PerspectiveHover = false;

		Lamp::Object* m_pSelectedObject = nullptr;
		bool m_InspectiorOpen = true;

		glm::vec2 m_MouseHoverPos = glm::vec2(0, 0);
		glm::vec2 m_WindowSize = glm::vec2(0, 0);
		Ref<Lamp::Shader> m_pShader;

		//Model importer
		bool m_ModelImporterOpen = false;
		Ref<Lamp::Model> m_pModelToImport;

		//Layers
		bool m_LayerViewOpen = true;

		//Play
		bool m_ShouldPlay = false;
		bool m_ShouldPlayPhysics = false;

		//Components
		bool m_AddComponentOpen = false;

		//Create
		bool m_CreateToolOpen = true;

		//Logginh
		bool m_LogToolOpen = false;
		//--------------------------------------
	};
}