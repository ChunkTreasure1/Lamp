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
				case Lamp::FramebufferTextureFormat::RED_INTEGER: return GL_R32I;
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
				case Lamp::FramebufferTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
			}
		}

		static GLint FormatToType(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case Lamp::FramebufferTextureFormat::None: return GL_UNSIGNED_BYTE;
				case Lamp::FramebufferTextureFormat::RGBA8: return GL_UNSIGNED_BYTE;
				case Lamp::FramebufferTextureFormat::RGBA16F: return GL_FLOAT;
				case Lamp::FramebufferTextureFormat::RGBA32F: return GL_FLOAT;
				case Lamp::FramebufferTextureFormat::RG32F: return GL_FLOAT;
				case Lamp::FramebufferTextureFormat::DEPTH32F: return GL_FLOAT;
				case Lamp::FramebufferTextureFormat::DEPTH24STENCIL8: return GL_UNSIGNED_BYTE;
				case Lamp::FramebufferTextureFormat::RED_INTEGER: return GL_INT;
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
		}

		static void AttachTexture(FramebufferSpecification spec, FramebufferTextureSpecification textureSpec, uint32_t id, bool multisample)
		{
			Utils::BindTexture(multisample, id);

			if (multisample)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.Samples, Utils::FormatToInternalGL(textureSpec.TextureFormat), spec.Width, spec.Height, GL_FALSE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, Utils::FormatToInternalGL(textureSpec.TextureFormat), spec.Width, spec.Height, 0,
					Utils::FormatToGL(textureSpec.TextureFormat), Utils::FormatToType(textureSpec.TextureFormat), nullptr);
			}

			if (!multisample)
			{
				glTexParameteri(Utils::TextureTarget(multisample), GL_TEXTURE_MIN_FILTER, Utils::FilteringToGL(textureSpec.TextureFiltering));
				glTexParameteri(Utils::TextureTarget(multisample), GL_TEXTURE_MAG_FILTER, Utils::FilteringToGL(textureSpec.TextureFiltering));

				glTexParameteri(Utils::TextureTarget(multisample), GL_TEXTURE_WRAP_S, Utils::WrapToGL(textureSpec.TextureWrap));
				glTexParameteri(Utils::TextureTarget(multisample), GL_TEXTURE_WRAP_T, Utils::WrapToGL(textureSpec.TextureWrap));

				glTexParameterfv(Utils::TextureTarget(multisample), GL_TEXTURE_BORDER_COLOR, glm::value_ptr(textureSpec.BorderColor));
			}
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case Lamp::FramebufferTextureFormat::DEPTH32F: return true;
			case Lamp::FramebufferTextureFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}

		static GLint GetAttachmentType(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case Lamp::FramebufferTextureFormat::DEPTH24STENCIL8: return GL_DEPTH_STENCIL_ATTACHMENT;
				case Lamp::FramebufferTextureFormat::DEPTH32F: return GL_DEPTH_ATTACHMENT;
			}
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		for (auto& format : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(format.TextureFormat))
			{
				m_ColorAttachmentSpecs.emplace_back(format);
			}
			else
			{
				m_DepthAttachmentFormatSpec = format;
			}
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);

		if (m_ColorAttachmentIDs.size())
		{
			glDeleteTextures(m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());
		}

		if (m_DepthAttachmentID != 0)
		{
			glDeleteTextures(1, &m_DepthAttachmentID);
		}
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

	void OpenGLFramebuffer::Resize(const uint32_t width, const uint32_t height)
	{
		if (width == 0 || height == 0)
		{
			LP_CORE_WARN("Attempted to resize to {0}, {1}", width, height);
			return;
		}

		m_Specification.Width = width;
		m_Specification.Height = height;

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

		glBlitNamedFramebuffer(rendererId, m_RendererID, 0, 0, size.x, size.y, 0, 0, size.x, size.y, bit, GL_LINEAR);
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

		glClearTexImage(m_ColorAttachmentIDs[attachmentIndex], 0, Utils::FormatToGL(att.TextureFormat), GL_INT, &value);
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
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			
			if (m_ColorAttachmentIDs.size())
			{
				glDeleteTextures(m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());
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

		bool multisample = m_Specification.Samples > 1;

		//Create attachments
		if (m_ColorAttachmentSpecs.size())
		{
			m_ColorAttachmentIDs.resize(m_ColorAttachmentSpecs.size());

			for (int i = 0; i < m_ColorAttachmentSpecs.size(); i++)
			{
				Utils::CreateTextures(multisample, m_ColorAttachmentIDs.data(), m_ColorAttachmentIDs.size());
			}

			for (size_t i = 0; i < m_ColorAttachmentIDs.size(); i++)
			{
				Utils::AttachTexture(m_Specification, m_ColorAttachmentSpecs[i], m_ColorAttachmentIDs[i], multisample);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, Utils::TextureTarget(multisample), m_ColorAttachmentIDs[i], 0);
			}
		}

		if (m_DepthAttachmentFormatSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			Utils::CreateTextures(multisample, &m_DepthAttachmentID, 1);
			Utils::AttachTexture(m_Specification, m_DepthAttachmentFormatSpec, m_DepthAttachmentID, multisample);
			glFramebufferTexture2D(GL_FRAMEBUFFER, Utils::GetAttachmentType(m_DepthAttachmentFormatSpec.TextureFormat), Utils::TextureTarget(multisample), m_DepthAttachmentID, 0);
		}

		if (m_ColorAttachmentSpecs.size() > 1)
		{
			std::vector<GLenum> buffers;
			for (int i = 0; i < m_ColorAttachmentSpecs.size(); i++)
			{
				buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
			}

			glDrawBuffers(buffers.size(), buffers.data());
		}
		else if (m_ColorAttachmentSpecs.empty())
		{
			glDrawBuffer(GL_NONE);
		}

		for (auto& buff : m_Specification.Attachments.Renderbuffers)
		{
			uint32_t id;
			glGenRenderbuffers(1, &id);
			glBindRenderbuffer(GL_RENDERBUFFER, id);

			glRenderbufferStorage(GL_RENDERBUFFER, Utils::RenderbufferTypeGL(buff.Format), m_Specification.Width, m_Specification.Height);
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