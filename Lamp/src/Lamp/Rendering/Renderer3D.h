#pragma once
#include "Cameras/PerspectiveCamera.h"
#include "Lamp/Meshes/Materials/Material.h"
#include "Lamp/Meshes/Mesh.h"

#include "Lamp/Core/GlobalEnvironment.h"
#include "Lamp/Rendering/Vertices/FrameBuffer.h"

namespace Lamp {
struct RenderPassInfo {
  Ref<CameraBase> Camera;
  DirectionalLight DirLight;
  glm::mat4 ViewProjection = glm::mat4(1.f);
  glm::mat4 LightViewProjection = glm::mat4(1.f);

  glm::vec4 ClearColor;
  bool IsShadowPass;
};

class Renderer3D {
public:
  static void Initialize();
  static void Shutdown();

  static void Begin(const RenderPassInfo &passInfo);
  static void End();
  static void Flush();

  static void DrawMesh(const glm::mat4 &modelMatrix, Ref<Mesh> &mesh,
                       Material &mat);
  static void DrawLine(const glm::vec3 &posA, const glm::vec3 &posB,
                       float width);

  static void DrawSkybox();
  static void DrawGrid();

  static Ref<FrameBuffer> &GetFrameBuffer() { return m_pFrameBuffer; }
  static Ref<FrameBuffer> &GetShadowBuffer() { return m_pShadowBuffer; }

private:
  static Ref<FrameBuffer> m_pFrameBuffer;
  static Ref<FrameBuffer> m_pShadowBuffer;

  static void StartNewBatch();
  static void ResetBatchData();
};
} // namespace Lamp