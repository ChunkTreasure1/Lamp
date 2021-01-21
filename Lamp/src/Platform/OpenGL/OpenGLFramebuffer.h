#pragma once

#include "Lamp/Rendering/Vertices/Framebuffer.h"

#include <functional>

namespace Lamp
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		
		virtual ~OpenGLFramebuffer() override;

		virtual void Bind() override;
		virtual void Unbind() override;
		virtual void Update(const uint32_t width, const uint32_t height) override;

		virtual const uint32_t GetColorAttachment() const override 
		{ 
			// TODO: Fix
			if (m_HasColorAttachment)
			{
				return m_AttachmentIDs[0];
			}
		}
		virtual const uint32_t GetDepthAttachment() const override 
		{ 
			// TODO: Fix
			if (m_HasColorAttachment)
			{
				return m_AttachmentIDs[1];
			}
			else
			{
				return m_AttachmentIDs[0];
			}
		}

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		void Invalidate();

	private:
		uint32_t m_RendererID;
		bool m_HasColorAttachment = false;

		std::vector<uint32_t> m_AttachmentIDs;
		FramebufferSpecification m_Specification;
	};
}