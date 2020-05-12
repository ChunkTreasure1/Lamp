#pragma once

#include <Lamp.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <glm/gtc/type_ptr.hpp>
#include <Lamp/Rendering/Cameras/PerspectiveCameraController.h>
#include <Lamp/Brushes/Brush.h>
#include <Lamp/Event/MouseEvent.h>

namespace Sandbox3D
{
	class Sandbox3D : public Lamp::Layer
	{
	public:
		Sandbox3D();

		virtual void Update(Lamp::Timestep ts) override;
		virtual void OnImGuiRender(Lamp::Timestep ts) override;
		virtual void OnEvent(Lamp::Event& e) override;

		virtual void OnItemClicked(Lamp::File& file) override;

	private:
		void CreateDockspace();
		void GetInput();
		void RenderGrid();

		bool OnMouseMoved(Lamp::MouseMovedEvent& e);

		//ImGui
		void UpdatePerspective();
		void UpdateAssetBrowser();
		void UpdateProperties();
		void UpdateModelImporter();

	private:
		Lamp::PerspectiveCameraController m_PCam;

		//------------TEST ENTITIES-------------
		Lamp::IEntity* m_pEntity;
		std::shared_ptr<Lamp::Texture2D> m_pTestTexture;

		Lamp::Brush* m_pBrush;
		//--------------------------------------

		//---------------Editor-----------------
		glm::vec3 m_FColor = glm::vec3{ 0.1f, 0.1f, 0.1f };
		glm::vec4 m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
		glm::vec2 m_PerspectiveSize = glm::vec2(0.f);
		ImGuiID m_DockspaceID;

		//Perspective
		const float m_AspectRatio = 1.7f;
		bool m_PerspectiveFocused = false;
		
		//Asset browser
		Lamp::File m_SelectedFile;
		int m_CurrSample = -1;
		bool m_AssetBrowserOpen = true;

		//Inspector
		bool m_MousePressed = false;
		bool m_PerspectiveHover = false;
		Lamp::IEntity* m_pSelectedEntity = nullptr;
		Lamp::Brush* m_pSelectedBrush = nullptr;
		bool m_InspectiorOpen = true;

		glm::vec2 m_MouseHoverPos = glm::vec2(0, 0);
		glm::vec2 m_WindowSize = glm::vec2(0, 0);
		std::shared_ptr<Lamp::Shader> m_pShader;

		//Model importer
		bool m_ModelImporterOpen = false;
		std::shared_ptr<Lamp::Model> m_pModelToImport;
		//--------------------------------------
	};
}