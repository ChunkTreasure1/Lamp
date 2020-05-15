#include "lppch.h"
#include "VertexBuffer.h"
#include <glad/glad.h>

namespace Lamp
{
	//////Vertex Buffer//////
	VertexBuffer::VertexBuffer(std::vector<Vertex>& pVertices, uint32_t size)
	{
		glGenBuffers(1, &m_RendererID);

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		if (size > 0)
		{
			glBufferData(GL_ARRAY_BUFFER, size, &pVertices[0], GL_DYNAMIC_DRAW);
		}
		else 
		{
			glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		}
	}

	VertexBuffer::VertexBuffer(uint32_t size)
	{
		glGenBuffers(1, &m_RendererID);
		
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	VertexBuffer::VertexBuffer(std::vector<float>& vertices, uint32_t size)
	{
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, &vertices[0], GL_STATIC_DRAW);
	}

	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void VertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void VertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VertexBuffer::SetVertices(std::vector<Vertex>& pVertices, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, &pVertices[0], GL_DYNAMIC_DRAW);
	}

	void VertexBuffer::SetData(const void* data, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	Ref<VertexBuffer> VertexBuffer::Create(std::vector<Vertex>& pVertices, uint32_t size)
	{
		return std::make_shared<VertexBuffer>(pVertices, size);
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		return std::make_shared<VertexBuffer>(size);
	}

	Ref<VertexBuffer> VertexBuffer::Create(std::vector<float>& vertices, uint32_t size)
	{
		return std::make_shared<VertexBuffer>(vertices, size);
	}

	//////Index Buffer//////

	IndexBuffer::IndexBuffer(std::vector<uint32_t>& pIndices, uint32_t count)
		: m_Count(count)
	{
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), &pIndices[0], GL_STATIC_DRAW);
	}

	IndexBuffer::IndexBuffer(uint32_t* pIndices, uint32_t count)
	{
		glGenBuffers(1, &m_RendererID);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), pIndices, GL_STATIC_DRAW);
	}

	IndexBuffer::~IndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void IndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void IndexBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	Ref<IndexBuffer> IndexBuffer::Create(std::vector<uint32_t>& pIndices, uint32_t size)
	{
		return std::make_shared<IndexBuffer>(pIndices, size);
	}
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* pIndices, uint32_t count)
	{
		return std::make_shared<IndexBuffer>(pIndices, count);
	}
}