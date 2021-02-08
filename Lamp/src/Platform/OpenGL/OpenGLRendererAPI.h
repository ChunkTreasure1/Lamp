#pragma once

#include "Lamp/Rendering//RendererAPI.h"

namespace Lamp {
class OpenGLRendererAPI : public RendererAPI {
public:
  virtual ~OpenGLRendererAPI() override = default;

  virtual void Initialize() override;
  virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width,
                           uint32_t height) override;
  virtual void SetClearColor(const glm::vec4 &color) override;
  virtual void Clear() override;
  virtual void ClearDepth() override;
  virtual void OffsetPolygon(float factor, float unit) override;

  virtual void DrawIndexed(const Ref<VertexArray> &vertexArray,
                           uint32_t = 0) override;
  virtual void DrawIndexedLines(const Ref<VertexArray> &vertexArray,
                                uint32_t indexCount) override;
};
} // namespace Lamp