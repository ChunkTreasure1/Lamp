#pragma once

#include "Sandbox/SandboxLayer.h"

#include "EditorWindow.h"

#include <Lamp/AssetSystem/MeshImporter/MeshImporter.h>

namespace Sandbox
{
	class MeshImporterPanel : public EditorWindow
	{
	public:
		MeshImporterPanel(std::string_view name);

		void OnEvent(Lamp::Event& e) override;
		inline const Ref<Lamp::PerspectiveCameraController>& GetCamera() { return m_camera; }

	private:
		void UpdatePerspective();
		void UpdateProperties();
		void UpdateMaterial();
		void UpdateCamera(Lamp::Timestep ts);

		void UpdateToolbar();
		void UpdateMeshConstruction();

		bool OnRender(Lamp::AppRenderEvent& e);
		void LoadMesh();
		void SaveMesh();

		bool OnUpdateImGui(Lamp::ImGuiUpdateEvent& e);
		bool OnUpdate(Lamp::AppUpdateEvent& e);

		std::string GetDragDropTarget();
		void MaterialPopup();

		bool m_hoveringPerspective = false;
		bool m_rightMousePressed = false;
		bool m_renderSkybox = false;
		bool m_renderGrid = true;
		glm::vec2 m_perspectiveSize;

		glm::mat4 m_transform = glm::mat4(1.f);
		glm::vec3 m_scale = glm::vec3(1.f);

		std::filesystem::path m_savePath;

		Ref<Lamp::Mesh> m_meshToImport;
		Ref<Lamp::PerspectiveCameraController> m_camera;
		Ref<Lamp::Shader> m_defaultShader;

		Ref<Lamp::RenderPipeline> m_renderPipeline;
		Ref<Lamp::Framebuffer> m_framebuffer;

		std::vector<Ref<Lamp::MaterialInstance>> m_materialInstances;
		std::vector<int> m_currentPipelinesSelected;

		//Icons
		Ref<Lamp::Texture2D> m_loadIcon;
		Ref<Lamp::Texture2D> m_saveIcon;

		//Import settings
		Lamp::MeshImportSettings m_importSettings;
	};
}