#include "lppch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Lamp
{
	static GLint FormatToInternalGL(FramebufferTextureFormat format)
	{
		switch (format)
		{
			case Lamp::FramebufferTextureFormat::None: return GL_NONE;
			case Lamp::FramebufferTextureFormat::RGBA8: return GL_RGBA8;
			case Lamp::FramebufferTextureFormat::RGBA16F: return GL_RGBA16F;
			case Lamp::FramebufferTextureFormat::RGBA32F: return GL_RGBA32F;
			case Lamp::FramebufferTextureFormat::RG32F: return GL_RG32F;
			case Lamp::FramebufferTextureFormat::DEPTH32F: return GL_DEPTH_COMPONENT;
			case Lamp::FramebufferTextureFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
		}
	}

	static GLint FormatToGL(FramebufferTextureFormat format)
	{
		switch (format)
		{
			case Lamp::FramebufferTextureFormat::None: return GL_NONE;
			case Lamp::FramebufferTextureFormat::RGBA8: return GL_RGBA;
			case Lamp::FramebufferTextureFormat::RGBA16F: return GL_RGBA;
			case Lamp::FramebufferTextureFormat::RGBA32F: return GL_RGBA;
			case Lamp::FramebufferTextureFormat::RG32F: return GL_RG;
			case Lamp::FramebufferTextureFormat::DEPTH32F: return GL_DEPTH_COMPONENT;
			case Lamp::FramebufferTextureFormat::DEPTH24STENCIL8: return GL_DEPTH_COMPONENT;
		}
	}

	static GLint FormatToType(FramebufferTextureFormat format)
	{
		switch (format)
		{
			case Lamp::FramebufferTextureFormat::None: return GL_UNSIGNED_INT;
			case Lamp::FramebufferTextureFormat::RGBA8: return GL_UNSIGNED_INT;
			case Lamp::FramebufferTextureFormat::RGBA16F: return GL_FLOAT;
			case Lamp::FramebufferTextureFormat::RGBA32F: return GL_FLOAT;
			case Lamp::FramebufferTextureFormat::RG32F: return GL_FLOAT;
			case Lamp::FramebufferTextureFormat::DEPTH32F: return GL_FLOAT;
			case Lamp::FramebufferTextureFormat::DEPTH24STENCIL8: return GL_UNSIGNED_INT;
		}
	}

	static GLint FilteringToGL(FramebufferTexureFiltering filtering)
	{
		switch (filtering)
		{
			case Lamp::FramebufferTexureFiltering::Nearest: return GL_NEAREST;
			case Lamp::FramebufferTexureFiltering::Linear: return GL_LINEAR;
			case Lamp::FramebufferTexureFiltering::NearestMipMapNearest: return GL_NEAREST_MIPMAP_NEAREST;
			case Lamp::FramebufferTexureFiltering::LinearMipMapNearest: return GL_LINEAR_MIPMAP_NEAREST;
			case Lamp::FramebufferTexureFiltering::NearestMipMapLinear: return GL_NEAREST_MIPMAP_LINEAR;
			case Lamp::FramebufferTexureFiltering::LinearMipMapLinear: return GL_LINEAR_MIPMAP_LINEAR;
		}
	}

	static GLint WrapToGL(FramebufferTextureWrap wrap)
	{
		switch (wrap)
		{
			case Lamp::FramebufferTextureWrap::Repeat: return GL_REPEAT;
			case Lamp::FramebufferTextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
			case Lamp::FramebufferTextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
			case Lamp::FramebufferTextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
			case Lamp::FramebufferTextureWrap::MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE;
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_AttachmentIDs.size(), m_AttachmentIDs.data());
	}

	void OpenGLFramebuffer::Bind()
	{
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Update(const uint32_t width, const uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	void OpenGLFramebuffer::Invalidate()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(m_AttachmentIDs.size(), m_AttachmentIDs.data());
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		int colorAtts = 0;
		for (auto& spec : m_Specification.Attachments.Attachments)
		{
			uint32_t id;
			glCreateTextures(GL_TEXTURE_2D, 1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			m_AttachmentIDs.push_back(id);

			glTexImage2D(GL_TEXTURE_2D, 0, FormatToInternalGL(spec.TextureFormat), m_Specification.Width, m_Specification.Height, 0, FormatToGL(spec.TextureFormat), FormatToType(spec.TextureFormat), nullptr);
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilteringToGL(spec.TextureFiltering));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilteringToGL(spec.TextureFiltering));

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapToGL(spec.TextureWrap));
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapToGL(spec.TextureWrap));

			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(spec.BorderColor));
			
			if (spec.TextureFormat == FramebufferTextureFormat::DEPTH32F || spec.TextureFormat == FramebufferTextureFormat::DEPTH24STENCIL8)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, id, 0);
			}
			else
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorAtts, GL_TEXTURE_2D, id, 0);
				colorAtts++;
				m_HasColorAttachment = true;
			}
		}

		//if (!m_HasColorAttachment)
		//{
		//	glDrawBuffer(GL_NONE);
		//	glReadBuffer(GL_NONE);
		//}
		//else
		//{
		//	glDrawBuffer(GL_FRONT);
		//}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}