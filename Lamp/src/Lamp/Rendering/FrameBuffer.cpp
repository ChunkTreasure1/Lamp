#include "lppch.h"
#include "FrameBuffer.h"

#include <glad/glad.h>

namespace Lamp
{
	FrameBuffer::FrameBuffer(const uint32_t width, const uint32_t height)
		: m_WindowWidth(width), m_WindowHeight(height)
	{
		glGenFramebuffers(1, &m_RendererID);
	
		glGenTextures(1, &m_Texture);
		glBindTexture(GL_TEXTURE_2D, m_Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_WindowWidth, m_WindowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
	}

	void FrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void FrameBuffer::Unbind()
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::Update(const uint32_t width, const uint32_t height)
	{
		m_WindowWidth = width;
		m_WindowHeight = height;
	}

	std::shared_ptr<FrameBuffer> FrameBuffer::Create(const uint32_t width, const uint32_t height)
	{
		return std::make_shared<FrameBuffer>(width, height);
	}
}