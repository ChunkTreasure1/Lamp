#pragma once

#include <functional>
#include "BufferLayout.h"
#include <glm/glm.hpp>

namespace Lamp
{
	struct Vertex
	{
		//Vertex()
		//{}

		//Vertex(const glm::vec3& pos, glm::vec3& normal, const glm::vec2& tex)
		//	: position(pos), normal(normal), textureCoords(tex)
		//{}

		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 textureCoords;
	};

	class VertexBuffer
	{
	public:
		VertexBuffer(std::vector<Vertex>& pVertices, uint32_t size);
		~VertexBuffer();

		void Bind() const;
		void Unbind() const;

		inline void SetBufferLayout(const BufferLayout& buff) { m_BufferLayout = buff; }
		inline BufferLayout& GetBufferLayout() { return m_BufferLayout; }
		void SetVertices(float* pVertices, uint32_t size);

	public:
		static VertexBuffer* Create(std::vector<Vertex>& pVertices, uint32_t size);

	private:
		uint32_t m_RendererID;
		BufferLayout m_BufferLayout;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(std::vector<uint32_t>& pIndices, uint32_t count);
		~IndexBuffer();

		void Bind() const;
		void Unbind() const;

		uint32_t GetCount() { return m_Count; }

	public:
		static IndexBuffer* Create(std::vector<uint32_t>& pIndices, uint32_t size);

	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};
}