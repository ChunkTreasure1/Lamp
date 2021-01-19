#include "lppch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace Lamp
{
	OpenGLFramebuffer::OpenGLFramebuffer(const uint32_t width, const uint32_t height, bool shadowBuf)
		: m_WindowWidth(width), m_WindowHeight(height)
	{
		m_IsShadowBuffer = shadowBuf;
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_ColorID);
		glDeleteTextures(1, &m_DepthID);
	}

	void OpenGLFramebuffer::Bind()
	{
		glViewport(0, 0, m_WindowWidth, m_WindowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Update(const uint32_t width, const uint32_t height)
	{
		m_WindowWidth = width;
		m_WindowHeight = height;

		Invalidate();
	}

	void OpenGLFramebuffer::Invalidate()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(1, &m_ColorID);
			glDeleteTextures(1, &m_DepthID);
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		if (m_IsShadowBuffer)
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthID);
			glBindTexture(GL_TEXTURE_2D, m_DepthID);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_WindowWidth, m_WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			float borderColor[] = { 1.f, 1.f, 1.f, 1.f };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthID, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		else
		{
			glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorID);
			glBindTexture(GL_TEXTURE_2D, m_ColorID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_WindowWidth, m_WindowHeight, 0, GL_RGBA, GL_UNSIGNED_INT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorID, 0);

			glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthID);
			glBindTexture(GL_TEXTURE_2D, m_DepthID);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_WindowWidth, m_WindowHeight);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthID, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDrawBuffer(GL_FRONT);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}