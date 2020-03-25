#include "lppch.h"
#include "FrameBuffer.h"

#include <glad/glad.h>

namespace Lamp
{
	FrameBuffer::FrameBuffer(const uint32_t width, const uint32_t height)
		: m_WindowWidth(width), m_WindowHeight(height)
	{
		glGenFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		// The texture we're going to render to
		glGenTextures(1, &m_Texture);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, m_Texture);

		// Give an empty image to OpenGL ( the last "0" means "empty" )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_WindowWidth, m_WindowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		// Poor filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenRenderbuffers(1, &m_DepthID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_WindowWidth, m_WindowHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthID);
	
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_Texture, 0);
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

		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_WindowWidth, m_WindowHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthID);

		glBindTexture(GL_TEXTURE_2D, m_Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_WindowWidth, m_WindowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	}

	std::shared_ptr<FrameBuffer> FrameBuffer::Create(const uint32_t width, const uint32_t height)
	{
		return std::make_shared<FrameBuffer>(width, height);
	}
}