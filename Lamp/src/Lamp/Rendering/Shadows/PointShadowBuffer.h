#pragma once

#include <functional>

#include "Lamp/Rendering/Vertices/Framebuffer.h"

namespace Lamp {
class PointShadowBuffer : Framebuffer {
public:
  PointShadowBuffer(const FramebufferSpecification &spec);
  virtual ~PointShadowBuffer() override;

  virtual void Bind() override;
  virtual void Unbind() override;
  virtual void Resize(uint32_t width, uint32_t height) override;
  virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override {
    return -1;
  }

  inline const std::vector<glm::mat4> &GetTransforms() { return m_Transforms; }

  virtual inline const uint32_t GetColorAttachmentID(uint32_t i) override {
    return 0;
  }
  virtual inline const uint32_t GetDepthAttachmentID() override {
    return m_TextureID;
  }
  virtual inline const uint32_t GetRendererID() override {
    return m_RendererID;
  }

  virtual void ClearAttachment(uint32_t attachmentIndex, int value) {}

  virtual void BindColorAttachment(uint32_t id /* = 0 */, uint32_t i /* = 0 */);
  virtual void BindDepthAttachment(uint32_t id /* = 0 */);

  virtual const FramebufferSpecification &GetSpecification() const {
    return FramebufferSpecification();
  }

  void SetPosition(const glm::vec3 &pos);
  inline const glm::vec3 &GetPosition() { return m_Position; }

private:
  void Invalidate();

private:
  std::vector<glm::mat4> m_Transforms;
  glm::mat4 m_Projection = glm::mat4(1.f);

  glm::vec3 m_Position = {0.f, 0.f, 0.f};

  uint32_t m_RendererID;
  uint32_t m_TextureID;

  FramebufferSpecification m_Specification;
};
} // namespace Lamp
