#pragma once

#include <Lamp.h>

#include "SandboxController.h"
#include "Sandbox/Actions/CommandStack.h"

#include "Windows/AssetBrowserPanel.h"
#include "Windows/CreatePanel.h"

#include "UI/AnimatedIcon.h"

#include <Lamp/Rendering/Cameras/PerspectiveCameraController.h>

#include <Lamp/Objects/Brushes/Brush.h>
#include <Lamp/Event/MouseEvent.h>
#include <Lamp/Event/EditorEvent.h>
#include <Lamp/Event/KeyEvent.h>
#include <Lamp/Rendering/RenderPass.h>

#include <Game/Game.h>

#include <ImGuizmo/ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

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
	class EditorWindow;

	class SandboxLayer : public Lamp::Layer
	{
	public:
		SandboxLayer();
		~SandboxLayer();

		bool OnUpdate(Lamp::AppUpdateEvent& e);

		void OnImGuiRender(Lamp::Timestep ts) override;
		void OnEvent(Lamp::Event& e) override;
		void OnRender() override; //TODO: fix so that it's not confused with app render event

	private:
		void CreateDockspace();
		void GetInput();
		void SetupFromConfig();

		void OnLevelPlay();
		void OnLevelStop();
		void OnSimulationStart();
		void OnSimulationStop();

		bool OnWindowClose(Lamp::WindowCloseEvent& e);
		bool OnMouseMoved(Lamp::MouseMovedEvent& e);
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

		Ref<Lamp::Level> m_runtimeLevel = nullptr;
		Ref<Lamp::Level> m_editLevel = nullptr;

		Ref<Lamp::Terrain> m_terrain;

		//---------------Editor-----------------
		glm::vec2 m_perspectiveSize = glm::vec2(0.f);
		glm::vec2 m_perspectiveBounds[2];
		ImGuiID m_dockspaceId;

		float m_assetManagerTime = 0.f;
		float m_createPanelTime = 0.f;
		float m_uiTotalTime = 0.f;

		Ref<Lamp::Texture2D> m_iconPlay;
		Ref<Lamp::Texture2D> m_iconStop;

		//Perspective
		bool m_perspectiveOpen = true;
		bool m_perspectiveFocused = false;
		bool m_RightMousePressed = false;
		CommandStack m_perspectiveCommands;

		ImGuizmo::OPERATION m_imGuizmoOperation = ImGuizmo::TRANSLATE;

		//Play
		AnimatedIcon m_physicsIcon;

		//Inspector
		bool m_perspectiveHover = false;
		bool m_inspectorOpen = true;

		Lamp::Object* m_pSelectedObject = nullptr;

		//Windows
		std::vector<EditorWindow*> m_windows;

		//Logging
		bool m_logToolOpen = false;

		//Level settings
		bool m_levelSettingsOpen = false;

		//Rendering settings
		bool m_RenderingSettingsOpen = false;

		enum class SceneState
		{
			Edit = 0,
			Play = 1,
			Simulating = 2
		};

		SceneState m_sceneState = SceneState::Edit;
		//--------------------------------------
	};
}