#pragma once

#include "Sandbox3D/Sandbox3D.h"

#include "BaseWindow.h"

namespace Sandbox3D
{
	class ModelImporter : public BaseWindow
	{
	public:
		ModelImporter(std::string_view name);

		virtual void OnEvent(Lamp::Event& e) override;
		inline const Ref<Lamp::PerspectiveCameraController>& GetCamera() { return m_Camera; }

	private:
		void RenderGrid();
		void UpdatePerspective();
		void UpdateProperties();
		void UpdateMaterial();
		void UpdateCamera(Lamp::Timestep ts);
		void Render();

		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool Update(Lamp::AppUpdateEvent& e);

	private:
		bool m_HoveringPerspective = false;
		glm::vec2 m_PerspectiveSize;
		std::string m_MaterialName = "";

		Ref<Lamp::Framebuffer> m_Framebuffer;
		Ref<Lamp::Model> m_pModelToImport;
		Ref<Lamp::PerspectiveCameraController> m_Camera;
		Ref<Lamp::Shader> m_DefaultShader;
	};
}