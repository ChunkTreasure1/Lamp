#pragma once

#include "BaseWindow.h"

namespace Lamp
{
	class Material;
}

namespace Sandbox
{
	class MaterialEditor : public BaseWindow
	{
	public:
		MaterialEditor(std::string_view name);

		void OnEvent(Lamp::Event& e) override;
	
	private:
		bool OnUpdate(Lamp::AppUpdateEvent& e);
		bool OnUpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnKeyPressed(Lamp::KeyPressedEvent& e);

		void UpdateMaterialView();
		void UpdateProperties();
		void UpdateMaterialList();
		void UpdateToolbar();

		void Render();
		void CreateNewMaterial();

	private:
		Ref<Lamp::Material> m_pSelectedMaterial = nullptr;
		Ref<Lamp::Framebuffer> m_framebuffer;
		Ref<Lamp::Mesh> m_materialModel;
		Ref<Lamp::PerspectiveCameraController> m_camera;
		Ref<Lamp::RenderGraph> m_renderGraph;
		std::filesystem::path m_pathOnRightClick;

		glm::vec2 m_perspectiveSize = { 0.f, 0.f };
		Ref<Lamp::Texture2D> m_saveIcon;
	};
}