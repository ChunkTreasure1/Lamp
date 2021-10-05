#include "lppch.h"
#include "OpenGLShaderStorageBuffer.h"

#include <glad/glad.h>

namespace Lamp
{
	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(uint32_t size, uint32_t binding)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
	}

	OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLShaderStorageBuffer::SetData(const void* data, uint32_t size)
	{
		glNamedBufferSubData(m_RendererID, 0, size, data);
	}

	void* OpenGLShaderStorageBuffer::Map()
	{
		return glMapNamedBuffer(m_RendererID, GL_READ_WRITE);
	}

	void OpenGLShaderStorageBuffer::Unmap()
	{
		glUnmapNamedBuffer(m_RendererID);
	}
}