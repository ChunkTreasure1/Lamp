#pragma once

#include "Sandbox3D/Sandbox3D.h"

namespace Sandbox3D {
class ModelImporter {
public:
  ModelImporter();

  void Update();
  void UpdateCamera(Lamp::Timestep ts);
  void Render();

  inline bool &GetIsOpen() { return m_Open; }
  void OnEvent(Lamp::Event &e);
  inline const Ref<Lamp::PerspectiveCameraController> &GetCamera() {
    return m_Camera;
  }

private:
  void RenderGrid();
  void UpdatePerspective();
  void UpdateProperties();
  void UpdateMaterial();

private:
  bool m_Open = false;
  bool m_HoveringPerspective = false;
  glm::vec2 m_PerspectiveSize;
  std::string m_MaterialName = "";

  Ref<Lamp::FrameBuffer> m_FrameBuffer;
  Ref<Lamp::Model> m_pModelToImport;
  Ref<Lamp::PerspectiveCameraController> m_Camera;
  Ref<Lamp::Shader> m_DefaultShader;
};
} // namespace Sandbox3D
