#include "lppch.h"
#include "OpenGLShaderStorageBuffer.h"

#include <glad/glad.h>

namespace Lamp
{
	namespace Utils
	{
		static GLuint DrawAccessToGLDrawAccess(DrawAccess access)
		{
			switch (access)
			{
				case Lamp::DrawAccess::Static: return GL_STATIC_DRAW;
				case Lamp::DrawAccess::Dynamic: return GL_DYNAMIC_DRAW;
			}

			return GL_DYNAMIC_DRAW;
		}
	}

	OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(uint32_t size, uint32_t binding, DrawAccess access)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, Utils::DrawAccessToGLDrawAccess(access));
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
		return nullptr;
	}
	void OpenGLShaderStorageBuffer::Unmap()
	{
	}
}