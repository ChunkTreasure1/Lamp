#include "lppch.h"
#include "PointShadowBuffer.h"

#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>

namespace Lamp
{
	PointShadowBuffer::PointShadowBuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		m_Projection = glm::perspective(glm::radians(90.f), (float)spec.width / (float)spec.height, 0.1f, 25.f);

		SetPosition(m_Position);

		//Invalidate();
	}

	PointShadowBuffer::~PointShadowBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(1, &m_TextureID);
	}

	void PointShadowBuffer::Bind()
	{
		glViewport(0, 0, m_Specification.width, m_Specification.height);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void PointShadowBuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void PointShadowBuffer::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.width = width;
		m_Specification.height = height;

		Invalidate();
		m_Projection = glm::perspective(glm::radians(90.f), (float)width / (float)height, m_NearPlane, m_FarPlane);
	}

	void PointShadowBuffer::BindColorAttachment(uint32_t id, uint32_t i)
	{

	}

	void PointShadowBuffer::BindDepthAttachment(uint32_t id)
	{
		glBindTextureUnit(id, m_TextureID);
	}

	void PointShadowBuffer::SetPosition(const glm::vec3& pos)
	{
		m_Position = pos;

		m_Transforms.clear();

		m_Transforms.push_back(m_Projection * glm::lookAt(m_Position, m_Position + glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)));
		m_Transforms.push_back(m_Projection * glm::lookAt(m_Position, m_Position + glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, -1.f, 0.f)));
		m_Transforms.push_back(m_Projection * glm::lookAt(m_Position, m_Position + glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f)));

		m_Transforms.push_back(m_Projection * glm::lookAt(m_Position, m_Position + glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, -1.f)));
		m_Transforms.push_back(m_Projection * glm::lookAt(m_Position, m_Position + glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, -1.f, 0.f)));
		m_Transforms.push_back(m_Projection * glm::lookAt(m_Position, m_Position + glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, -1.f, 0.f)));
	}

	void PointShadowBuffer::UpdateProjection()
	{
		m_Projection = glm::perspective(glm::radians(90.f), (float)m_Specification.width / (float)m_Specification.height, m_NearPlane, m_FarPlane);
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
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, m_Specification.width, m_Specification.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
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

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}