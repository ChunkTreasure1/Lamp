#pragma once

#include "Lamp/Rendering/Vertices/FrameBuffer.h"

namespace Lamp {
class ReflectionBuffer : Framebuffer {
public:
  ReflectionBuffer();

  virtual ~ReflectionBuffer() override {}

  virtual void Bind() override {}
  virtual void Unbind() override {}
  virtual void Resize(uint32_t width, uint32_t height) override {}
  virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override {
    return -1;
  }

  virtual inline const uint32_t GetColorAttachmentID(uint32_t i) override {
    return 0;
  }
  virtual inline const uint32_t GetDepthAttachmentID() override { return 0; }
  virtual inline const uint32_t GetRendererID() override {
    return m_RendererId;
  }

  virtual void ClearAttachment(uint32_t attachmentIndex, int value) {}

  virtual void BindColorAttachment(uint32_t id /* = 0 */,
                                   uint32_t i /* = 0 */) {}
  virtual void BindDepthAttachment(uint32_t id /* = 0 */) {}

  virtual const FramebufferSpecification &GetSpecification() const {
    return FramebufferSpecification();
  }

  void Generate();

private:
  glm::vec3 m_Position;
  glm::vec3 m_BoxSize;

  glm::mat4 m_Projection;
  std::vector<glm::mat4> m_Views;

  uint32_t m_RendererId;
  uint32_t m_TextureId;

  uint32_t m_Width = 256;
  uint32_t m_Height = 256;
};
} // namespace Lamp