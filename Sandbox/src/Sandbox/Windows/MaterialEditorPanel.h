#pragma once

#include "EditorWindow.h"

#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/Event/KeyEvent.h>

namespace Lamp
{
	class Material;
	class Mesh;
	class Framebuffer;
	class PerspectiveCameraController;
	class Texture2D;
}

namespace Sandbox
{
	class MaterialEditorPanel : public EditorWindow
	{
	public:
		MaterialEditorPanel(std::string_view name);

		void OnEvent(Lamp::Event& e) override;
	
	private:
		bool OnRender(Lamp::AppRenderEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);
		bool OnUpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnKeyPressed(Lamp::KeyPressedEvent& e);

		void UpdateMaterialView();
		void UpdateProperties();
		void UpdateMaterialList();
		void UpdateToolbar();

		void CreateNewMaterial();

	private:
		Ref<Lamp::Material> m_selectedMaterial = nullptr;
		Ref<Lamp::MaterialInstance> m_materialInstance = nullptr;
		
		Ref<Lamp::Mesh> m_materialModel;
		
		Ref<Lamp::Framebuffer> m_framebuffer;
		Ref<Lamp::RenderPipeline> m_renderPipeline;
		Ref<Lamp::PerspectiveCameraController> m_camera;
		
		glm::vec2 m_perspectiveSize = { 0.f, 0.f };
		
		std::string m_searchQuery;
		
		Ref<Lamp::Texture2D> m_saveIcon;
		Ref<Lamp::Texture2D> m_reloadIcon;
		Ref<Lamp::Texture2D> m_searchIcon;
	};
}