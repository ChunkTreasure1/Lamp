#pragma once

#include "BufferLayout.h"
#include <functional>
#include <glm/glm.hpp>

#include "Lamp/Core/Core.h"

namespace Lamp {
struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::vec2 textureCoords;
};

class VertexBuffer {
public:
  virtual ~VertexBuffer() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual void SetBufferLayout(const BufferLayout &buff) = 0;
  virtual BufferLayout &GetBufferLayout() = 0;

  virtual void SetVertices(std::vector<Vertex> &pVertices, uint32_t size) = 0;
  virtual void SetData(const void *data, uint32_t size) = 0;

public:
  static Ref<VertexBuffer> Create(std::vector<Vertex> &pVertices,
                                  uint32_t size);
  static Ref<VertexBuffer> Create(uint32_t size);
  static Ref<VertexBuffer> Create(std::vector<float> &vertices, uint32_t size);
};

class IndexBuffer {
public:
  virtual ~IndexBuffer() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual uint32_t GetCount() = 0;

public:
  static Ref<IndexBuffer> Create(std::vector<uint32_t> &pIndices,
                                 uint32_t count);
  static Ref<IndexBuffer> Create(uint32_t *pIndices, uint32_t count);
};
} // namespace Lamp
