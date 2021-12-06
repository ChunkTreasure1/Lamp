#include "lppch.h"
#include "OpenGLFramebuffer.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Lamp
{
	namespace Utils
	{
		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outId, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), count, outId);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static GLint FormatToInternalGL(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::None: return GL_NONE;
				case ImageFormat::RGBA: return GL_RGBA8;
				case ImageFormat::RGBA16F: return GL_RGBA16F;
				case ImageFormat::RGBA32F: return GL_RGBA32F;
				case ImageFormat::RG32F: return GL_RG32F;
				case ImageFormat::R32I: return GL_R32I;
				case ImageFormat::R32F: return GL_RED;
				case ImageFormat::DEPTH32F: return GL_DEPTH_COMPONENT;
				case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
			}

			return GL_NONE;
		}

		static GLint FormatToGL(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::None: return GL_NONE;
				case ImageFormat::RGBA: return GL_RGBA;
				case ImageFormat::RGBA16F: return GL_RGBA;
				case ImageFormat::RGBA32F: return GL_RGBA;
				case ImageFormat::RG32F: return GL_RG;
				case ImageFormat::R32F: return GL_RED;
				case ImageFormat::DEPTH32F: return GL_DEPTH_COMPONENT;
				case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH_COMPONENT;
				case ImageFormat::R32I: return GL_RED_INTEGER;
			}

			return GL_NONE;
		}

		static GLint FormatToType(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::None: return GL_UNSIGNED_BYTE;
				case ImageFormat::RGBA: return GL_UNSIGNED_BYTE;
				case ImageFormat::RGBA16F: return GL_FLOAT;
				case ImageFormat::RGBA32F: return GL_FLOAT;
				case ImageFormat::RG32F: return GL_FLOAT;
				case ImageFormat::R32F: return GL_FLOAT;
				case ImageFormat::DEPTH32F: return GL_FLOAT;
				case ImageFormat::DEPTH24STENCIL8: return GL_UNSIGNED_BYTE;
				case ImageFormat::R32I: return GL_INT;
			}

			return GL_NONE;
		}

		static GLint FilteringToGL(TextureFilter filtering)
		{
			switch (filtering)
			{
				case TextureFilter::Nearest: return GL_NEAREST;
				case TextureFilter::Linear: return GL_LINEAR;
			}

			return GL_NONE;
		}

		static GLint WrapToGL(TextureWrap wrap)
		{
			switch (wrap)
			{
				case TextureWrap::Repeat: return GL_REPEAT;
				case TextureWrap::Clamp: return GL_CLAMP_TO_BORDER;
			}

			return GL_NONE;
		}

		static GLint RenderbufferTypeGL(FramebufferRenderbufferType type)
		{
			switch (type)
			{
				case Lamp::FramebufferRenderbufferType::Color: return GL_COLOR_COMPONENTS;
				case Lamp::FramebufferRenderbufferType::Depth: return GL_DEPTH_COMPONENT;
					break;
				default:
					break;
			}

			return GL_NONE;
		}

		static GLint RenderbufferTypeAttachment(FramebufferRenderbufferType type)
		{
			switch (type)
			{
				case Lamp::FramebufferRenderbufferType::Color: return GL_COLOR_ATTACHMENT0;
				case Lamp::FramebufferRenderbufferType::Depth: return GL_DEPTH_ATTACHMENT;
				default:
					break;
			}

			return GL_NONE;
		}

		static void AttachTexture(FramebufferSpecification spec, FramebufferTextureSpecification textureSpec, uint32_t id, bool multisample)
		{
			Utils::BindTexture(multisample, id);

			if (multisample)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.samples, Utils::FormatToInternalGL(textureSpec.textureFormat), spec.width, spec.height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, Utils::FormatToInternalGL(textureSpec.textureFormat), spec.width, spec.height, 0,
					Utils::FormatToGL(textureSpec.textureFormat), Utils::FormatToType(textureSpec.textureFormat), nullptr);
			}

			if (!multisample)
			{
				glTexParameteri(Utils::TextureTarget(multisample), GL_TEXTURE_MIN_FILTER, Utils::FilteringToGL(textureSpec.textureFiltering));
				glTexParameteri(Utils::TextureTarget(multisample), GL_TEXTURE_MAG_FILTER, Utils::FilteringToGL(textureSpec.textureFiltering));

				glTexParameteri(Utils::TextureTarget(multisample), GL_TEXTURE_WRAP_T, Utils::WrapToGL(textureSpec.textureWrap));

				glTexParameterfv(Utils::TextureTarget(multisample), GL_TEXTURE_BORDER_COLOR, glm::value_ptr(textureSpec.BorderColor));
			}
		}

		static GLint GetAttachmentType(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH_STENCIL_ATTACHMENT;
				case ImageFormat::DEPTH32F: return GL_DEPTH_ATTACHMENT;
			}

			return 0;
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

		if (m_ColorAttachmentIDs.size())
		{
			glDeleteTextures((GLsizei)m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());
		}

		if (m_DepthAttachmentID != 0)
		{
			glDeleteTextures(1, &m_DepthAttachmentID);
		}
	}

	void OpenGLFramebuffer::Bind()
	{
		glViewport(0, 0, m_Specification.width, m_Specification.height);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(const uint32_t width, const uint32_t height)
	{
		if (width == 0 || height == 0)
		{
			LP_CORE_WARN("Attempted to resize to {0}, {1}", width, height);
			return;
		}

		m_Specification.width = width;
		m_Specification.height = height;

		Invalidate();
	}

	int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		Bind();
		LP_CORE_ASSERT(attachmentIndex < m_ColorAttachmentIDs.size(), "Index out of range!");
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);

		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);

		return pixelData;
	}

	void OpenGLFramebuffer::Copy(uint32_t rendererId, const glm::vec2& size, bool depth)
	{
		int bit = depth ? GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT;

		glBlitNamedFramebuffer(rendererId, m_RendererID, 0, 0, (GLint)size.x, (GLint)size.y, 0, 0, (GLint)size.x, (GLint)size.y, bit, GL_NEAREST);
	}

	inline const uint32_t OpenGLFramebuffer::GetColorAttachmentID(uint32_t i)
	{
		LP_CORE_ASSERT(i < m_ColorAttachmentIDs.size(), "Index out of range!");
		return m_ColorAttachmentIDs[i];
	}

	inline const uint32_t OpenGLFramebuffer::GetDepthAttachmentID()
	{
		return m_DepthAttachmentID;
	}

	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		auto& att = m_ColorAttachmentSpecs[attachmentIndex];

		glClearTexImage(m_ColorAttachmentIDs[attachmentIndex], 0, Utils::FormatToGL(att.textureFormat), GL_INT, &value);
	}

	void OpenGLFramebuffer::BindColorAttachment(uint32_t id, uint32_t i)
	{
		glBindTextureUnit(id, m_ColorAttachmentIDs[i]);
	}

	void OpenGLFramebuffer::BindDepthAttachment(uint32_t id)
	{
		glBindTextureUnit(id, m_DepthAttachmentID);
	}

	void OpenGLFramebuffer::Invalidate()
	{
		m_ColorAttachmentSpecs.clear();
		for (auto& format : m_Specification.attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(format.textureFormat))
			{
				m_ColorAttachmentSpecs.emplace_back(format);
			}
			else
			{
				m_DepthAttachmentFormatSpec = format;
			}
		}

		if (m_RendererID > 0)
		{
			glDeleteFramebuffers(1, &m_RendererID);

			if (m_ColorAttachmentIDs.size())
			{
				glDeleteTextures((GLsizei)m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());
			}

			if (m_DepthAttachmentID != 0)
			{
				glDeleteTextures(1, &m_DepthAttachmentID);
			}

			m_ColorAttachmentIDs.clear();
			m_DepthAttachmentID = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool multisample = m_Specification.samples > 1;

		//Create attachments
		if (m_ColorAttachmentSpecs.size())
		{
			m_ColorAttachmentIDs.resize(m_ColorAttachmentSpecs.size());

			Utils::CreateTextures(multisample, m_ColorAttachmentIDs.data(), (uint32_t)m_ColorAttachmentIDs.size());

			for (size_t i = 0; i < m_ColorAttachmentIDs.size(); i++)
			{
				Utils::AttachTexture(m_Specification, m_ColorAttachmentSpecs[i], m_ColorAttachmentIDs[i], multisample);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, Utils::TextureTarget(multisample), m_ColorAttachmentIDs[i], (GLenum)0);
			}
		}

		if (m_DepthAttachmentFormatSpec.textureFormat != ImageFormat::None)
		{
			Utils::CreateTextures(multisample, &m_DepthAttachmentID, 1);
			Utils::AttachTexture(m_Specification, m_DepthAttachmentFormatSpec, m_DepthAttachmentID, multisample);
			glFramebufferTexture2D(GL_FRAMEBUFFER, Utils::GetAttachmentType(m_DepthAttachmentFormatSpec.textureFormat), Utils::TextureTarget(multisample), m_DepthAttachmentID, 0);
		}

		if (m_ColorAttachmentSpecs.size() > 1)
		{
			std::vector<GLenum> buffers;
			for (int i = 0; i < m_ColorAttachmentSpecs.size(); i++)
			{
				buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
			}

			glDrawBuffers((GLsizei)buffers.size(), buffers.data());
		}
		else if (m_ColorAttachmentSpecs.empty())
		{
			glDrawBuffer(GL_NONE);
		}

		for (auto& buff : m_Specification.attachments.Renderbuffers)
		{
			uint32_t id;
			glGenRenderbuffers(1, &id);
			glBindRenderbuffer(GL_RENDERBUFFER, id);

			glRenderbufferStorage(GL_RENDERBUFFER, Utils::RenderbufferTypeGL(buff.Format), m_Specification.width, m_Specification.height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, Utils::RenderbufferTypeAttachment(buff.Format), GL_RENDERBUFFER, id);

			m_RenderbufferIDs.push_back(id);
		}

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			LP_CORE_ERROR("Framebuffer error: {0}", status);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}