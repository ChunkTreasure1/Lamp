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

		void UpdateMaterialView();
		void UpdateProperties();
		void UpdateMaterialList();

		void Render();

	private:
		Lamp::Material* m_pSelectedMaterial = nullptr;
		Ref<Lamp::Framebuffer> m_Framebuffer;
		Ref<Lamp::Model> m_MaterialModel;
		Ref<Lamp::PerspectiveCameraController> m_Camera;
	};
}