#include "lppch.h"
#include "PointShadowBuffer.h"

#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>

namespace Lamp
{
	PointShadowBuffer::PointShadowBuffer(uint32_t width, uint32_t height, const glm::vec3& pos)
		: m_Width(width), m_Height(height), m_Position(pos)
	{
		Invalidate();

		m_Projection = glm::perspective(glm::radians(90.f), (float)width / (float)height, 0.1f, 25.f);
		m_Transforms.push_back(m_Projection * glm::lookAt(pos, pos + glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)));
		m_Transforms.push_back(m_Projection * glm::lookAt(pos, pos + glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)));
		m_Transforms.push_back(m_Projection * glm::lookAt(pos, pos + glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)));
		
		m_Transforms.push_back(m_Projection * glm::lookAt(pos, pos + glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)));
		m_Transforms.push_back(m_Projection * glm::lookAt(pos, pos + glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)));
		m_Transforms.push_back(m_Projection * glm::lookAt(pos, pos + glm::vec3(1.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f)));
	}

	void PointShadowBuffer::Bind()
	{
	}

	void PointShadowBuffer::Update(uint32_t width, uint32_t height)
	{
		m_Width = width;
		m_Height = height;
	}

	void PointShadowBuffer::Invalidate()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(1, &m_TextureID);
		}

		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_TextureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureID);

		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_TextureID, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}