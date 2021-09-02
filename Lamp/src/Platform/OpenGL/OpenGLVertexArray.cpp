#include "lppch.h"
#include "OpenGLVertexArray.h"

#include <glad/glad.h>

namespace Lamp
{

	static GLenum ElementTypeToGLEnum(Type type)
	{
		switch (type)
		{
			case Lamp::Type::Bool: return GL_BOOL;
			case Lamp::Type::Int: return GL_INT;
			case Lamp::Type::Float: return GL_FLOAT;
			case Lamp::Type::Float2: return GL_FLOAT;
			case Lamp::Type::Float3: return GL_FLOAT;
			case Lamp::Type::Float4: return GL_FLOAT;
			case Lamp::Type::Mat3: return GL_FLOAT;
			case Lamp::Type::Mat4: return GL_FLOAT;
		}
	}

	OpenGLVertexArray::OpenGLVertexArray()
		: m_NumAttributes(0)
	{
		glGenVertexArrays(1, &m_RendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void OpenGLVertexArray::Bind()
	{
		glBindVertexArray(m_RendererID);
	}

	void OpenGLVertexArray::Unbind()
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& pVertexBuffer, Ref<Shader> shader)
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

	void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& pIndexBuffer)
	{
		glBindVertexArray(m_RendererID);
		pIndexBuffer->Bind();

		m_IndexBuffer = pIndexBuffer;
	}
}