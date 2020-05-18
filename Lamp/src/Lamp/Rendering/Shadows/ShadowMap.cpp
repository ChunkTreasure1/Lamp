#include "lppch.h"
#include "ShadowMap.h"

#include <glad/glad.h>

namespace Lamp
{
	ShadowMap::ShadowMap(const uint32_t width, const uint32_t height)
	{
		m_Width = width;
		m_Height = height;

		glGenFramebuffers(1, &m_RendererID);

		glGenTextures(1, &m_DepthID);
		glBindTexture(GL_TEXTURE_2D, m_DepthID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthID, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		Unbind();
	}

	void ShadowMap::Bind()
	{
		glViewport(0, 0, m_Width, m_Height);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void ShadowMap::Unbind()
	{
	}

	std::shared_ptr<ShadowMap> ShadowMap::Create(const uint32_t width, const uint32_t height)
	{
		return std::make_shared<ShadowMap>(width, height);
	}
}