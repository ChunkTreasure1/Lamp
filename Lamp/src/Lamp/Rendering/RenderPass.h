#pragma once

#include "Lamp/Core/Core.h"
#include "RenderCommand.h"
#include "Renderer3D.h"
#include "Vertices/Framebuffer.h"

#define LP_EXTRA_RENDER(fn) std::bind(&fn, this)

namespace Lamp {
struct RenderPassSpecification {
  using RenderFunc = std::function<void()>;
  std::vector<RenderFunc> ExtraRenders;

  Ref<CameraBase> Camera;
  Ref<Framebuffer> TargetFramebuffer;

  bool IsShadowPass = false;
  bool IsPointShadowPass = false;
  uint32_t LightIndex = 0;
};

class RenderPass {
public:
  friend class RenderPassManager;

  RenderPass(const RenderPassSpecification &spec);
  ~RenderPass() {}

  inline uint32_t GetID() { return m_ID; }
  inline const RenderPassSpecification &GetSpecification() const {
    return m_PassSpec;
  }

  void Render();

private:
  inline void SetID(uint32_t id) { m_ID = id; }

private:
  uint32_t m_ID;
  RenderPassSpecification m_PassSpec;
};

class RenderPassManager {
public:
  RenderPassManager() = default;
  ~RenderPassManager() {
    m_RenderPasses.clear();
    s_Instance = nullptr;
  }

  void AddPass(Ref<RenderPass> &pass);
  bool RemovePass(uint32_t id);

  void RenderPasses();

public:
  static Ref<RenderPassManager> &Get() {
    if (s_Instance.get() == nullptr) {
      s_Instance = CreateRef<RenderPassManager>();
    }
    return s_Instance;
  }

private:
  std::vector<Ref<RenderPass>> m_RenderPasses;

private:
  static Ref<RenderPassManager> s_Instance;
};
} // namespace Lamp
