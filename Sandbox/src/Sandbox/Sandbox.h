#pragma once

#include <Lamp.h>

#include "SandboxController.h"

#include "Actions/ActionHandler.h"

#include "Windows/AssetBrowserPanel.h"
#include "Windows/CreatePanel.h"

#include "UI/AnimatedIcon.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <Lamp/Rendering/Cameras/PerspectiveCameraController.h>

#include <Lamp/Objects/Brushes/Brush.h>
#include <Lamp/Event/MouseEvent.h>
#include <Lamp/Event/EditorEvent.h>
#include <Lamp/Event/KeyEvent.h>
#include <Lamp/Rendering/RenderPass.h>

#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo/ImGuizmo.h>
#include <Game/Game.h>

namespace Lamp
{
	class Framebuffer;
	class RenderPipeline;
	class Skybox;
	class Terrain;

	struct RenderPass;
}

namespace Sandbox
{
	class BaseWindow;

	class Sandbox : public Lamp::Layer
	{
	public:
		Sandbox();
		~Sandbox();

		bool OnUpdate(Lamp::AppUpdateEvent& e);

		void OnImGuiRender(Lamp::Timestep ts) override;
		void OnEvent(Lamp::Event& e) override;
		void OnRender() override; //TODO: fix so that it's not confused with app render event

	private:
		void CreateDockspace();
		void SetupRenderPasses();
		void GetInput();
		void SetupFromConfig();

		void OnLevelPlay();
		void OnLevelStop();
		void OnSimulationStart();
		void OnSimulationStop();

		bool OnMouseMoved(Lamp::MouseMovedEvent& e);
		bool OnWindowClose(Lamp::WindowCloseEvent& e);
		bool OnKeyPressed(Lamp::KeyPressedEvent& e);
		bool OnImGuiBegin(Lamp::ImGuiBeginEvent& e);
		bool OnViewportSizeChanged(Lamp::EditorViewportSizeChangedEvent& e);
		bool OnObjectSelected(Lamp::EditorObjectSelectedEvent& e);

		//ImGui
		void UpdatePerspective();
		void UpdateProperties();
		void UpdateAddComponent();
		void UpdateLogTool();
		bool DrawComponent(Lamp::EntityComponent* ptr);
		void UpdateLevelSettings();
		void UpdateRenderingSettings();
		void UpdateToolbar();
		void UpdateStatistics();

		//Shortcuts
		void SaveLevelAs();
		void OpenLevel();
		void OpenLevel(const std::filesystem::path& path);
		void NewLevel();
		void Undo();
		void Redo();

	private:
		Scope<Game> m_pGame;
		Ref<SandboxController> m_sandboxController;
		Ref<Lamp::Framebuffer> m_SelectionBuffer;

		Ref<Lamp::Level> m_pLevel = nullptr;
		Ref<Lamp::Level> m_pRuntimeLevel = nullptr;
		 
		Ref<Lamp::Terrain> m_terrain;

		//---------------Editor-----------------
		glm::vec3 m_FColor = glm::vec3{ 0.1f, 0.1f, 0.1f };
		glm::vec4 m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
		glm::vec2 m_perspectiveSize = glm::vec2(0.f);
		glm::vec2 m_perspectiveBounds[2];
		ImGuiID m_DockspaceID;

		float m_assetManagerTime = 0.f;
		float m_createPanelTime = 0.f;
		float m_uiTotalTime = 0.f;

		Ref<Lamp::Texture2D> m_IconPlay;
		Ref<Lamp::Texture2D> m_IconStop;

		Ref<Lamp::Framebuffer> m_depthPrePassFramebuffer;
		Ref<Lamp::Framebuffer> m_viewportFramebuffer;
		Ref<Lamp::Framebuffer> m_geometryFramebuffer;
		Ref<Lamp::Framebuffer> m_ssaoMainFramebuffer;

		//Perspective
		bool m_perspectiveOpen = true;
		bool m_perspectiveFocused = false;
		bool m_RightMousePressed = false;

		ImGuizmo::OPERATION m_ImGuizmoOperation = ImGuizmo::TRANSLATE;
		ActionHandler m_ActionHandler;

		//Play
		AnimatedIcon m_PhysicsIcon;

		//Asset browser
		AssetBrowserPanel m_assetManager;
		CreatePanel m_createPanel;

		//Inspector
		bool m_MousePressed = false;
		bool m_perspectiveHover = false;

		Lamp::Object* m_pSelectedObject = nullptr;
		bool m_inspectorOpen = true;

		glm::vec2 m_MouseHoverPos = glm::vec2(0, 0);
		glm::vec2 m_WindowSize = glm::vec2(0, 0);

		//Windows
		std::vector<BaseWindow*> m_pWindows;

		//Components
		bool m_AddComponentOpen = false;

		//Logging
		bool m_logToolOpen = false;

		//Level settings
		bool m_LevelSettingsOpen = false;

		//Rendering settings
		bool m_RenderingSettingsOpen = false;
		bool m_RenderPassViewOpen = false;
		bool m_ShaderViewOpen = false;

		enum class SceneState
		{
			Edit = 0,
			Play = 1,
			Simulating = 2
		};

		SceneState m_SceneState = SceneState::Edit;
		//--------------------------------------
	};
}