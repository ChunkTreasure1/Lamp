#pragma once

#include "BaseWindow.h"

namespace Lamp
{
	class Material;
}

namespace Sandbox3D
{
	class MaterialEditor : public BaseWindow
	{
	public:
		MaterialEditor(std::string_view name);

		virtual void OnEvent(Lamp::Event& e);
	
	private:
		bool OnUpdate(Lamp::AppUpdateEvent& e);
		bool OnUpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnKeyPressed(Lamp::KeyPressedEvent& e);

		void UpdateMaterialView();
		void UpdateProperties();
		void UpdateMaterialList();
		void UpdateRightClickMenu();

		std::string GetDragDropTarget();

		void Render();
		void CreateNewMaterial();

	private:
		Lamp::Material* m_pSelectedMaterial = nullptr;
		Ref<Lamp::Framebuffer> m_Framebuffer;
		Ref<Lamp::Mesh> m_MaterialModel;
		Ref<Lamp::PerspectiveCameraController> m_Camera;
		std::filesystem::path m_pathOnRightClick;

		glm::vec2 m_perspectiveSize = { 0.f, 0.f };
	};
}