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
		virtual void Resize(const uint32_t width, const uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual inline const uint32_t GetColorAttachmentID(uint32_t i /* = 0 */) override;
		virtual inline const uint32_t GetDepthAttachmentID() override;
		virtual inline const uint32_t GetRendererID() override { return m_RendererID; }

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual void BindColorAttachment(uint32_t id, uint32_t i);
		virtual void BindDepthAttachment(uint32_t id);

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		void Invalidate();

	private:
		uint32_t m_RendererID;

		std::vector<uint32_t> m_ColorAttachmentIDs;
		uint32_t m_DepthAttachmentID;

		FramebufferSpecification m_Specification;
		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
		FramebufferTextureSpecification m_DepthAttachmentFormatSpec = FramebufferTextureFormat::None;
	};
}