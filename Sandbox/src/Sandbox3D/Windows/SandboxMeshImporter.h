#pragma once

#include "Sandbox3D/Sandbox3D.h"

#include "BaseWindow.h"

namespace Sandbox3D
{
	struct ImportSettings
	{
		float MeshScale = 1.f;
		glm::vec3 MeshUp = { 0.f, 1.f, 0.f };
	};

	class SandboxMeshImporter : public BaseWindow
	{
	public:
		SandboxMeshImporter(std::string_view name);

		virtual void OnEvent(Lamp::Event& e) override;
		inline const Ref<Lamp::PerspectiveCameraController>& GetCamera() { return m_Camera; }

	private:
		void RenderGrid();
		void UpdatePerspective();
		void UpdateProperties();
		void UpdateMaterial();
		void UpdateCamera(Lamp::Timestep ts);

		void UpdateToolbar();
		void UpdateMeshConstruction();

		void Render();

		bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool Update(Lamp::AppUpdateEvent& e);

		std::string GetDragDropTarget();
		void MaterialPopup();

	private:
		bool m_HoveringPerspective = false;
		bool m_RightMousePressed = false;
		bool m_RenderSkybox = false;
		bool m_RenderGrid = true;
		glm::vec2 m_PerspectiveSize;
		std::string m_SavePath = "";
		std::string m_SourcePath = "";

		Ref<Lamp::Framebuffer> m_Framebuffer;
		Ref<Lamp::Mesh> m_pModelToImport;
		Ref<Lamp::PerspectiveCameraController> m_Camera;
		Ref<Lamp::Shader> m_DefaultShader;

		std::vector<int> m_ShaderSelectionIds;

		//Icons
		Ref<Lamp::Texture2D> m_LoadIcon;
		Ref<Lamp::Texture2D> m_SaveIcon;

		//Import settings
		ImportSettings m_ImportSettings;
	};
}