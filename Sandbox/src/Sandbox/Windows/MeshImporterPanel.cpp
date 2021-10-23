#include "MeshImporterPanel.h"

#include <Lamp/Mesh/Materials/MaterialLibrary.h>
#include <Lamp/Rendering/RenderCommand.h>
#include <Lamp/Rendering/Shader/ShaderLibrary.h>
#include <Lamp/Utility/PlatformUtility.h>
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

#include <Lamp/AssetSystem/AssetManager.h>
#include <Lamp/AssetSystem/ResourceCache.h>
#include <Lamp/Rendering/RenderGraph/Nodes/RenderNodeEnd.h>
#include <Lamp/Rendering/RenderGraph/RenderGraph.h>
#include <Platform/OpenGL/OpenGLFramebuffer.h>

#include <Lamp/Utility/UIUtility.h>

namespace Sandbox {
using namespace Lamp;

MeshImporterPanel::MeshImporterPanel(std::string_view name) : BaseWindow(name) {
  m_camera = CreateRef<PerspectiveCameraController>(60.f, 0.01f, 100.f);
  m_camera->SetPosition({-3.f, 2.f, 3.f});

  m_RenderFuncs.emplace_back(LP_EXTRA_RENDER(MeshImporterPanel::Render));

  // Setup icons
  m_loadIcon = ResourceCache::GetAsset<Texture2D>(
      "engine/textures/ui/MeshImporter/loadIcon.png");
  m_saveIcon = ResourceCache::GetAsset<Texture2D>(
      "engine/textures/ui/MeshImporter/saveIcon.png");

  m_renderGraph =
      ResourceCache::GetAsset<RenderGraph>("assets/editor.rendergraph");
  m_renderGraph->Start();

  m_framebuffer = m_renderGraph->GetSpecification().endNode->framebuffer;
}

bool MeshImporterPanel::UpdateImGui(Lamp::ImGuiUpdateEvent &e) {
  if (!m_IsOpen) {
    return false;
  }

  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin(m_Name.c_str(), &m_IsOpen);
  ImGui::PopStyleVar();

  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID(m_Name.c_str());
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  ImGui::End();

  UpdatePerspective();
  UpdateProperties();
  UpdateMaterial();
  UpdateToolbar();
  UpdateMeshConstruction();

  return false;
}

bool MeshImporterPanel::Update(Lamp::AppUpdateEvent &e) {
  if (m_defaultShader == nullptr) {
    m_defaultShader = Lamp::ShaderLibrary::GetShader("pbrEditor");
  }
  UpdateCamera(e.GetTimestep());

  return false;
}

std::string MeshImporterPanel::GetDragDropTarget() {
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *pPayload =
            ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
      const wchar_t *path = (const wchar_t *)pPayload->Data;
      std::filesystem::path p = std::filesystem::path("assets") / path;
      return p.string();
    }

    ImGui::EndDragDropTarget();
  }

  return "";
}

void MeshImporterPanel::MaterialPopup() {}

void MeshImporterPanel::UpdateCamera(Lamp::Timestep ts) {
  if (!m_IsOpen) {
    return;
  }

  if (Input::IsMouseButtonPressed(1) &&
      (m_hoveringPerspective || m_rightMousePressed)) {
    m_camera->Update(ts);
  }
}

void MeshImporterPanel::UpdateToolbar() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                        ImVec4(0.3f, 0.305f, 0.31f, 0.5f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                        ImVec4(0.5f, 0.505f, 0.51f, 0.5f));

  ImGui::Begin("##toolbarMeshImporter", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoScrollWithMouse);

  float size = ImGui::GetWindowHeight() - 4.f;

  if (ImGui::ImageButton((ImTextureID)m_loadIcon->GetID(), {size, size},
                         {0.f, 1.f}, {1.f, 0.f}, 0)) {
    m_importSettings.path = FileDialogs::OpenFile("FBX File (*.fbx)\0*.fbx\0");
    if (!m_importSettings.path.empty() &&
        std::filesystem::exists(m_importSettings.path)) {
      LoadMesh();
    }
  }

  ImGui::SameLine();

  if (ImGui::ImageButton((ImTextureID)m_saveIcon->GetID(), {size, size},
                         {0.f, 1.f}, {1.f, 0.f}, 0)) {
    m_savePath = Lamp::FileDialogs::SaveFile("Lamp Geometry (*.lgf)\0*.lgf\0");

    std::string matOriginPath = m_savePath.substr(
        m_savePath.find_last_of('\\') + 1, m_savePath.size() - 1);

    if (!m_savePath.empty()) {
      if (m_savePath.find(".lgf") == std::string::npos) {
        m_savePath += ".lgf";
      }

      m_modelToImport->Path = m_savePath;

      for (auto &mat : m_modelToImport->GetMaterials()) {
        mat.second->SetName(matOriginPath + std::to_string(mat.first));
        if (!MaterialLibrary::IsMaterialLoaded(matOriginPath +
                                               std::to_string(mat.first))) {
          std::string matPath =
              m_savePath.substr(0, m_savePath.find_last_of('\\') + 1);
          matPath += matOriginPath + ".mtl";

          mat.second->Path = std::filesystem::path(matPath);
          g_pEnv->pAssetManager->SaveAsset(mat.second);

          MaterialLibrary::AddMaterial(mat.second);
        }
      }

      g_pEnv->pAssetManager->SaveAsset(m_modelToImport);
      m_savePath = "";
    }
  }

  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor(3);
  ImGui::End();
}

void MeshImporterPanel::UpdateMeshConstruction() {}

void MeshImporterPanel::Render() {
  if (!m_IsOpen) {
    return;
  }

  if (m_modelToImport) {
    for (const auto &mesh : m_modelToImport->GetSubMeshes()) {
      Renderer3D::SubmitMesh(
          glm::mat4(1.f), mesh,
          m_modelToImport->GetMaterials()[mesh->GetMaterialIndex()]);
    }
  }

  m_renderGraph->Run(m_camera->GetCamera());
}

void MeshImporterPanel::LoadMesh() {
  m_modelToImport = MeshImporter::ImportMesh(m_importSettings);

  m_savePath = m_importSettings.path.stem().string();
  m_savePath += ".lgf";
  m_modelToImport->Path = m_savePath;

  for (auto &mat : m_modelToImport->GetMaterials()) {
    mat.second->SetShader(m_defaultShader);

    for (auto &tex : mat.second->GetTextures()) {
      tex.second = ResourceCache::GetAsset<Texture2D>(
          "engine/textures/default/defaultTexture.png");
    }
  }

  m_shaderSelectionIds.clear();
  for (int i = 0; i < m_modelToImport->GetMaterials().size(); i++) {
    m_shaderSelectionIds.push_back(0);
  }
}

void MeshImporterPanel::OnEvent(Lamp::Event &e) {
  if (!m_IsOpen) {
    return;
  }

  if (Input::IsMouseButtonReleased(1)) {
    m_camera->SetHasControl(false);
    m_rightMousePressed = false;

    Application::Get().GetWindow().ShowCursor(true);
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
  }

  if (Input::IsMouseButtonPressed(1) &&
      (m_hoveringPerspective || m_rightMousePressed)) {
    m_camera->OnEvent(e);
    m_rightMousePressed = true;

    Application::Get().GetWindow().ShowCursor(false);
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
  }

  EventDispatcher dispatcher(e);
  dispatcher.Dispatch<Lamp::ImGuiUpdateEvent>(
      LP_BIND_EVENT_FN(MeshImporterPanel::UpdateImGui));
  dispatcher.Dispatch<Lamp::AppUpdateEvent>(
      LP_BIND_EVENT_FN(MeshImporterPanel::Update));
}

void MeshImporterPanel::UpdatePerspective() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("Import Perspective");
  {
    glm::vec2 windowPos =
        glm::vec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
    m_hoveringPerspective = ImGui::IsWindowHovered();
    m_camera->SetControlsEnabled(m_hoveringPerspective);

    if (m_renderGraph->GetSpecification().endNode->framebuffer) {
      ImVec2 panelSize = ImGui::GetContentRegionAvail();
      if (m_perspectiveSize != *((glm::vec2 *)&panelSize)) {
        m_renderGraph->GetSpecification().endNode->framebuffer->Resize(
            (uint32_t)panelSize.x, (uint32_t)panelSize.y);
        m_perspectiveSize = {panelSize.x, panelSize.y};

        m_camera->UpdateProjection((uint32_t)panelSize.x,
                                   (uint32_t)panelSize.y);
      }

      uint32_t textureID = m_renderGraph->GetSpecification()
                               .endNode->framebuffer->GetColorAttachmentID();
      ImGui::Image((void *)(uint64_t)textureID,
                   ImVec2{panelSize.x, panelSize.y}, ImVec2{0, 1},
                   ImVec2{1, 0});
    }
  }
  ImGui::End();
  ImGui::PopStyleVar();
}

void MeshImporterPanel::UpdateProperties() {
  ImGui::Begin("Import Settings", &m_IsOpen);

  ImGui::Text(("Source path: " + m_importSettings.path.string()).c_str());
  ImGui::Text(("Destination path: " + m_savePath).c_str());

  UI::BeginProperties("importProps");

  UI::Property("Show Skybox", m_renderSkybox);
  UI::Property("Show Grid", m_renderGrid);

  UI::EndProperties();

  ImGui::Separator();
  ImGui::Text("Mesh settings");

  static std::vector<const char *> units = {"Centimeters", "Decimeters",
                                            "Meters"};
  static int currentUnit = 0;

  static std::vector<const char *> meshDirections = {"Y+ up", "Y- up", "Z+ up",
                                                     "Z- up", "X+ up", "X- up"};
  static int currentDirection = 0;

  if (UI::BeginProperties("meshSettings")) {
    if (UI::Combo("Units", currentUnit, units)) {
      m_importSettings.units = static_cast<Units>(currentUnit);
      LoadMesh();
    }

    if (UI::Combo("Up direction", currentDirection, meshDirections)) {
    }

    UI::EndProperties();
  }

  ImGui::End();
}

void MeshImporterPanel::UpdateMaterial() {
  ImGui::Begin("Materials");

  static std::vector<const char *> shaders;
  static std::unordered_map<std::string, std::string> paths;

  shaders.clear();
  for (auto &shader : ShaderLibrary::GetShaders()) {
    shaders.push_back(shader->GetName().c_str());
  }

  if (m_modelToImport.get()) {
    int matId = 0;
    for (auto &mat : m_modelToImport->GetMaterials()) {
      for (int i = 0; i < shaders.size(); i++) {
        if (mat.second->GetShader()->GetName() == shaders[i]) {
          m_shaderSelectionIds[matId] = i;
        }
      }
      matId++;
    }

    int i = 0;
    for (auto &mat : m_modelToImport->GetMaterials()) {
      std::string id = mat.second->GetName() + "###mat" + std::to_string(i);
      if (ImGui::CollapsingHeader(id.c_str())) {
        std::string matName = mat.second->GetName();

        std::string propId = "##props" + std::to_string(i);
        if (UI::BeginProperties(propId)) {
          if (UI::Property("Name", matName)) {
            mat.second->SetName(matName);
          }

          if (UI::Combo("Shader", m_shaderSelectionIds[i], shaders)) {
            if (mat.second->GetShader() !=
                ShaderLibrary::GetShader(shaders[m_shaderSelectionIds[i]])) {
              mat.second->SetShader(
                  ShaderLibrary::GetShader(shaders[m_shaderSelectionIds[i]]));
            }
          }

          UI::EndProperties();
        }

        ImGui::Separator();

        for (auto &tex : mat.second->GetTextures()) {
          ImGui::Text(tex.first.c_str());
          if (ImGui::ImageButton((ImTextureID)tex.second->GetID(), {64, 64},
                                 {0, 1}, {1, 0})) {
          }
          if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload *pPayload =
                    ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
              const wchar_t *wPath = (const wchar_t *)pPayload->Data;
              std::filesystem::path path(wPath);
              AssetType type =
                  g_pEnv->pAssetManager->GetAssetTypeFromPath(path);
              if (type == AssetType::Texture) {
                tex.second = ResourceCache::GetAsset<Texture2D>(
                    std::filesystem::path("assets") / path);
              }
            }

            ImGui::EndDragDropTarget();
          }
          ImGui::Separator();
        }
      }

      i++;
    }
  }

  ImGui::End();
}
} // namespace Sandbox
