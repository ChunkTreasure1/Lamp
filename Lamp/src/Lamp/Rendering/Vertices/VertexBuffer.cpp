#include "lppch.h"
#include "VertexBuffer.h"
#include <glad/glad.h>

namespace Lamp
{
	//////Vertex Buffer//////
	VertexBuffer::VertexBuffer(float* pVertices, uint32_t size)
	{
		glGenBuffers(1, &m_RendererID);

		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, pVertices, GL_DYNAMIC_DRAW);
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

	void VertexBuffer::SetVertices(float* pVertices, uint32_t size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		glBufferData(GL_ARRAY_BUFFER, size, pVertices, GL_DYNAMIC_DRAW);
	}

	VertexBuffer* VertexBuffer::Create(float* pVertices, uint32_t size)
	{
		return new VertexBuffer(pVertices, size);
	}

	//////Index Buffer//////

	IndexBuffer::IndexBuffer(uint32_t* pIndices, uint32_t count)
		: m_Count(count)
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

	IndexBuffer* IndexBuffer::Create(uint32_t* pIndices, uint32_t size)
	{
		return new IndexBuffer(pIndices, size);
	}
}