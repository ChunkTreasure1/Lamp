#pragma once

#include "Sandbox/Sandbox.h"

#include "BaseWindow.h"

#include <Lamp/AssetSystem/MeshImporter.h>

namespace Sandbox
{
class MeshImporterPanel : public BaseWindow
{
public:
    MeshImporterPanel(std::string_view name);

    void OnEvent(Lamp::Event& e) override;
    inline const Ref<Lamp::PerspectiveCameraController>& GetCamera() {
        return m_camera;
    }

private:
    void UpdatePerspective();
    void UpdateProperties();
    void UpdateMaterial();
    void UpdateCamera(Lamp::Timestep ts);

    void UpdateToolbar();
    void UpdateMeshConstruction();

    void Render();
    void LoadMesh();

    bool UpdateImGui(Lamp::ImGuiUpdateEvent& e);
    bool Update(Lamp::AppUpdateEvent& e);

    std::string GetDragDropTarget();
    void MaterialPopup();

private:
    bool m_hoveringPerspective = false;
    bool m_rightMousePressed = false;
    bool m_renderSkybox = false;
    bool m_renderGrid = true;
    glm::vec2 m_perspectiveSize;
    std::string m_savePath;

    Ref<Lamp::Mesh> m_modelToImport;
    Ref<Lamp::PerspectiveCameraController> m_camera;
    Ref<Lamp::Shader> m_defaultShader;
    Ref<Lamp::Framebuffer> m_framebuffer;

    std::vector<int> m_shaderSelectionIds;

    //Icons
    Ref<Lamp::Texture2D> m_loadIcon;
    Ref<Lamp::Texture2D> m_saveIcon;
    Ref<Lamp::RenderGraph> m_renderGraph;

    //Import settings
    Lamp::ImportSettings m_importSettings;
};
}