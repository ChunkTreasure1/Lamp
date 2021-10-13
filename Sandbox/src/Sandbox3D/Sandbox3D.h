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

#include "Actions/ActionHandler.h"
#include "Windows/BufferWindow.h"
#include "Windows\AssetManagerPanel.h"
#include "Lamp/Event/EditorEvent.h"

#include "UI/AnimatedIcon.h"

#include <Game/Game.h>

namespace Lamp
{
	class Framebuffer;
}

namespace Sandbox3D
{
	class BaseWindow;

	class Sandbox3D : public Lamp::Layer
	{
	public:
		Sandbox3D();
		~Sandbox3D();

		bool OnUpdate(Lamp::AppUpdateEvent& e);

		virtual void OnImGuiRender(Lamp::Timestep ts) override;
		virtual void OnEvent(Lamp::Event& e) override;

	private:
		void CreateDockspace();
		void GetInput();
		void RenderGrid();
		void RenderSkybox();
		void SetupFromConfig();
		void RenderLines();

		void OnLevelPlay();
		void OnLevelStop();
		void OnSimulationStart();
		void OnSimulationStop();

		bool OnMouseMoved(Lamp::MouseMovedEvent& e);
		bool OnWindowClose(Lamp::WindowCloseEvent& e);
		bool OnKeyPressed(Lamp::KeyPressedEvent& e);
		bool OnImGuiBegin(Lamp::ImGuiBeginEvent& e);
		bool OnViewportSizeChanged(Lamp::EditorViewportSizeChangedEvent& e);

		//ImGui
		void UpdatePerspective();
		void UpdateProperties();
		void UpdateAddComponent();
		void UpdateCreateTool();
		void UpdateLogTool();
		bool DrawComponent(Lamp::EntityComponent* ptr);
		void UpdateLevelSettings();
		void UpdateRenderingSettings();
		void UpdateRenderPassView();
		void UpdateShaderView();
		void UpdateToolbar();

		//Shortcuts
		void SaveLevelAs();
		void OpenLevel();
		void OpenLevel(const std::filesystem::path& path);
		void NewLevel();
		void Undo();
		void Redo();

	private:
		Scope<Game> m_pGame;
		Ref<SandboxController> m_SandboxController;
		Ref<Lamp::Framebuffer> m_SelectionBuffer;

		Ref<Lamp::Level> m_pLevel = nullptr;
		Ref<Lamp::Level> m_pRuntimeLevel = nullptr;

		std::vector<std::pair<glm::vec3, glm::vec3>> m_Lines;

		//---------------Editor-----------------
		glm::vec3 m_FColor = glm::vec3{ 0.1f, 0.1f, 0.1f };
		glm::vec4 m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
		glm::vec2 m_PerspectiveSize = glm::vec2(0.f);
		glm::vec2 m_PerspectiveBounds[2];
		ImGuiID m_DockspaceID;
		std::vector<BufferWindow> m_BufferWindows;

		Ref<Lamp::Texture2D> m_IconPlay;
		Ref<Lamp::Texture2D> m_IconStop;

		//Perspective
		bool m_PerspectiveOpen = true;
		const float m_AspectRatio = 1.7f;
		bool m_PerspectiveFocused = false;
		bool m_HaveUndone = false;
		bool m_IsPlaying = false;
		bool m_RightMousePressed = false;

		ImGuizmo::OPERATION m_ImGuizmoOperation = ImGuizmo::TRANSLATE;
		ActionHandler m_ActionHandler;

		//Asset browser
		AssetManagerPanel m_assetManager;

		//Inspector
		bool m_MousePressed = false;
		bool m_PerspectiveHover = false;

		Lamp::Object* m_pSelectedObject = nullptr;
		bool m_InspectiorOpen = true;

		glm::vec2 m_MouseHoverPos = glm::vec2(0, 0);
		glm::vec2 m_WindowSize = glm::vec2(0, 0);

		//Windows
		std::vector<BaseWindow*> m_pWindows;

		//Play
		AnimatedIcon m_PhysicsIcon;

		//Components
		bool m_AddComponentOpen = false;

		//Create
		bool m_CreateToolOpen = true;

		//Logging
		bool m_LogToolOpen = false;

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