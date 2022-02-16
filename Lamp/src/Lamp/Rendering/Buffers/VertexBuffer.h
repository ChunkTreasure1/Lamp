#pragma once

#include "BufferLayout.h"

#include "Lamp/Rendering/Vertex.h"

#include <glm/glm.hpp>
#include <functional>

namespace Lamp
{
	class CommandBuffer;
	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Bind(Ref<CommandBuffer> commandBuffer) const = 0;
		virtual void Unbind() const = 0;

		virtual void SetBufferLayout(const BufferLayout& buff) = 0;
		virtual BufferLayout& GetBufferLayout() = 0;

		virtual void SetVertices(std::vector<Vertex>& pVertices, uint32_t size) = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;

	public:
		static Ref<VertexBuffer> Create(const std::vector<Vertex>& pVertices, uint32_t size);
		static Ref<VertexBuffer> Create(const std::vector<float>& vertices, uint32_t size);
		static Ref<VertexBuffer> Create(uint32_t size);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void Bind(Ref<CommandBuffer> commandBuffer) const = 0;

		virtual void Unbind() const = 0;

		virtual uint32_t GetCount() = 0;

	public:
		static Ref<IndexBuffer> Create(const std::vector<uint32_t>& pIndices, uint32_t count);
		static Ref<IndexBuffer> Create(uint32_t* pIndices, uint32_t count);

	};
}