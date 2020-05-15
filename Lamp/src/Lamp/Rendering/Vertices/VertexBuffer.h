#pragma once

#include <functional>
#include "BufferLayout.h"
#include <glm/glm.hpp>

#include "Lamp/Core/Core.h"

namespace Lamp
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 textureCoords;
	};

	class VertexBuffer
	{
	public:
		VertexBuffer(std::vector<Vertex>& pVertices, uint32_t size);
		VertexBuffer(uint32_t size);
		VertexBuffer(std::vector<float>& vertices, uint32_t size);

		~VertexBuffer();

		void Bind() const;
		void Unbind() const;

		inline void SetBufferLayout(const BufferLayout& buff) { m_BufferLayout = buff; }
		inline BufferLayout& GetBufferLayout() { return m_BufferLayout; }
		void SetVertices(std::vector<Vertex>& pVertices, uint32_t size);
		void SetData(const void* data, uint32_t size);

	public:
		static Ref<VertexBuffer> Create(std::vector<Vertex>& pVertices, uint32_t size);
		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(std::vector<float>& vertices, uint32_t size);

	private:
		uint32_t m_RendererID;
		BufferLayout m_BufferLayout;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(std::vector<uint32_t>& pIndices, uint32_t count);
		IndexBuffer(uint32_t* pIndices, uint32_t count);
		~IndexBuffer();

		void Bind() const;
		void Unbind() const;

		uint32_t GetCount() { return m_Count; }

	public:
		static Ref<IndexBuffer> Create(std::vector<uint32_t>& pIndices, uint32_t count);
		static Ref<IndexBuffer> Create(uint32_t* pIndices, uint32_t count);

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}