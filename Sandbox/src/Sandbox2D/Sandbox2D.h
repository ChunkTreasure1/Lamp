#pragma once

#include <Lamp.h>

#include <glm/glm.hpp>

namespace Sandbox2D
{
	class Sandbox2D : public Lamp::Layer 
	{
	public:
		Sandbox2D();

		bool OnUpdate(Lamp::AppUpdateEvent& e);
		virtual void OnImGuiRender(Lamp::Timestep ts) override;
		virtual void OnEvent(Lamp::Event& e) override;

		virtual void OnItemClicked(Lamp::File& file) override;

	private:
		void GetInput();
		void UpdateDockspace();
		bool MouseMoved(Lamp::MouseMovedEvent& e);

		void RenderPerspective();
		void RenderAssetBrowser();
		void RenderProperties();
		void RenderLayerView();
		void RenderLog();

	private:
		Lamp::OrthographicCameraController* m_CameraController;

		//---------------Editor-----------------
		glm::vec3 m_FColor = glm::vec3{ 0.1f, 0.1f, 0.1f };
		glm::vec4 m_ClearColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.f);
		glm::vec2 m_PerspectiveSize = glm::vec2(0.f);
		ImGuiID m_DockspaceID;

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

		//Layers
		bool m_LayerViewOpen = true;
		//--------------------------------------
	};
}