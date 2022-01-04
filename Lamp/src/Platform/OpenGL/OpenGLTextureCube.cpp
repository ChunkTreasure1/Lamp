#include "lppch.h"
#include "OpenGLTextureCube.h"
#include "Lamp/Rendering/Buffers/Framebuffer.h"

#include <glad/glad.h>

namespace Lamp
{
	OpenGLTextureCube::OpenGLTextureCube(const std::filesystem::path& path)
	{
	}

	OpenGLTextureCube::OpenGLTextureCube(ImageFormat format, uint32_t width, uint32_t height)
	{
		glGenTextures(1, &m_rendererId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererId);
		for (uint32_t i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererId);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	OpenGLTextureCube::~OpenGLTextureCube()
	{
		glDeleteTextures(1, &m_rendererId);
	}

	void OpenGLTextureCube::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_rendererId);
	}

	void OpenGLTextureCube::SetData(const void* data, uint32_t size)
	{
	}
}