#include "lppch.h"
#include "VertexArray.h"

#include <glad/glad.h>
namespace Lamp
{
	static GLenum ElementTypeToGLEnum(ElementType type)
	{
		switch (type)
		{
			case Lamp::ElementType::Bool: return GL_BOOL;
			case Lamp::ElementType::Int: return GL_INT;
			case Lamp::ElementType::Float: return GL_FLOAT;
			case Lamp::ElementType::Float2: return GL_FLOAT;
			case Lamp::ElementType::Float3: return GL_FLOAT;
			case Lamp::ElementType::Float4: return GL_FLOAT;
			case Lamp::ElementType::Mat3: return GL_FLOAT;
			case Lamp::ElementType::Mat4: return GL_FLOAT;
		}
	}

	VertexArray::VertexArray()
	{
		glGenVertexArrays(1, &m_RendererID);
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void VertexArray::Bind()
	{
		glBindVertexArray(m_RendererID);
	}

	void VertexArray::Unbind()
	{
		glBindVertexArray(0);
	}

	void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& pVertexBuffer)
	{
		glBindVertexArray(m_RendererID);
		pVertexBuffer->Bind();

		for (auto& element : pVertexBuffer->GetBufferLayout().GetElements())
		{
			glVertexAttribPointer(m_NumAttributes, 
								  element.GetComponentCount(element.ElementType), 
								  ElementTypeToGLEnum(element.ElementType), 
								  element.Normalized ? GL_TRUE : GL_FALSE, 
								  pVertexBuffer->GetBufferLayout().GetStride(), 
								  (const void*)element.Offset);
			glEnableVertexAttribArray(m_NumAttributes);
			m_NumAttributes++;
		}

		m_VertexBuffers.push_back(pVertexBuffer);
	}

	void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& pIndexBuffer)
	{
		glBindVertexArray(m_RendererID);
		pIndexBuffer->Bind();

		m_pIndexBuffer = pIndexBuffer;
	}
	std::shared_ptr<VertexArray> VertexArray::Create()
	{
		return std::make_shared<VertexArray>();
	}
}