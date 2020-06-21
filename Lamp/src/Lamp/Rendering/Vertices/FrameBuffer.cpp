#include "lppch.h"
#include "FrameBuffer.h"

#include <glad/glad.h>

namespace Lamp
{
	FrameBuffer::FrameBuffer(const uint32_t width, const uint32_t height)
		: m_WindowWidth(width), m_WindowHeight(height)
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(1, &m_ColorID);
			glDeleteTextures(1, &m_DepthID);
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorID);
		glBindTexture(GL_TEXTURE_2D, m_ColorID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_WindowWidth, m_WindowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorID, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthID);
		glBindTexture(GL_TEXTURE_2D, m_DepthID);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_WindowWidth, m_WindowHeight);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthID, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_ColorID);
		glDeleteTextures(1, &m_DepthID);
	}

	void FrameBuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_WindowWidth, m_WindowHeight);
	}

	void FrameBuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FrameBuffer::Update(const uint32_t width, const uint32_t height)
	{
		m_WindowWidth = width;
		m_WindowHeight = height;

		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(1, &m_ColorID);
			glDeleteTextures(1, &m_DepthID);
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorID);
		glBindTexture(GL_TEXTURE_2D, m_ColorID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_WindowWidth, m_WindowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorID, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthID);
		glBindTexture(GL_TEXTURE_2D, m_DepthID);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, m_WindowWidth, m_WindowHeight);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthID, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	std::shared_ptr<FrameBuffer> FrameBuffer::Create(const uint32_t width, const uint32_t height)
	{
		return std::make_shared<FrameBuffer>(width, height);
	}
}