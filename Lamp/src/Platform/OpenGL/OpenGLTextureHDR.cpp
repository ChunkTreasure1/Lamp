#include "lppch.h"
#include "OpenGLTextureHDR.h"

#include <glad/glad.h>
#include <stb/stb_image.h>

namespace Lamp
{
	OpenGLTextureHDR::OpenGLTextureHDR(const std::filesystem::path& path)
	{
		int width, height, channels;
		float* pData = stbi_loadf(path.string().c_str(), &width, &height, &channels, 0);

		glGenTextures(1, &m_rendererId);
		glBindTexture(GL_TEXTURE_2D, m_rendererId);

		//Set texture wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//Set filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (pData)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, pData);
			stbi_image_free(pData);
		}
		else
		{
			SetFlag(AssetFlag::Missing);
		}

		if (channels == 4)
		{
			m_internalFormat = GL_RGBA8;
			m_dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			m_internalFormat = GL_RGB8;
			m_dataFormat = GL_RGB;
		}
		else if (channels == 1)
		{
			m_internalFormat = GL_R8;
			m_dataFormat = GL_RED;
		}

		m_width = width;
		m_height = height;
	}

	OpenGLTextureHDR::OpenGLTextureHDR(uint32_t width, uint32_t height)
	{
	}

	OpenGLTextureHDR::~OpenGLTextureHDR()
	{
		glDeleteTextures(1, &m_rendererId);
	}

	void OpenGLTextureHDR::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_rendererId);
	}

	void OpenGLTextureHDR::SetData(const void* data, uint32_t size)
	{
		glTextureSubImage2D(m_rendererId, 0, 0, 0, m_width, m_height, m_dataFormat, GL_FLOAT, data);
	}
}