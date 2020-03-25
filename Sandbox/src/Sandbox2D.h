#pragma once

#include <Lamp.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <glm/gtc/type_ptr.hpp>
#include <Lamp/Rendering/Cameras/PerspectiveCameraController.h>
#include <Lamp/Brushes/Brush.h>

namespace Sandbox2D
{
	class Sandbox2D : public Lamp::Layer
	{
	public:
		Sandbox2D();

		virtual void Update(Lamp::Timestep ts) override;
		virtual void OnImGuiRender(Lamp::Timestep ts) override;
		virtual void OnEvent(Lamp::Event& e) override;

		virtual void OnItemClicked(Lamp::File& file) override;

	private:
		void CreateDockspace();
		bool OnMouseMoved(Lamp::MouseMovedEvent& e);

		//ImGui
		void UpdatePerspective();
		void UpdateAssetBrowser();
		void UpdateProperties();

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
		
		//Asset browser
		Lamp::File m_SelectedFile;
		int m_CurrSample = -1;

		//Inspector
		bool m_MousePressed = false;
		bool m_PerspectiveHover = false;
		Lamp::IEntity* m_pSelectedEntity = nullptr;

		glm::vec2 m_MouseHoverPos = glm::vec2(0, 0);
		//--------------------------------------
	};
}