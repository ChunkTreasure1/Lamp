#pragma once

#include "Lamp/Rendering/Vertices/FrameBuffer.h"

#include <functional>

namespace Lamp
{
	class OpenGLFramebuffer : public FrameBuffer
	{
	public:
		OpenGLFramebuffer(const uint32_t width, const uint32_t height, bool shadowBuf = false);
		
		virtual ~OpenGLFramebuffer() override;

		virtual void Bind() override;
		virtual void Unbind() override;
		virtual void Update(const uint32_t width, const uint32_t height) override;

		virtual inline const uint32_t GetColorAttachment() const override { return m_ColorID; }
		virtual inline const uint32_t GetDepthAttachment() const override { return m_DepthID; }

	private:
		void Invalidate();

	private:
		uint32_t m_RendererID;
		uint32_t m_DepthID;
		uint32_t m_ColorID;

		uint32_t m_WindowWidth;
		uint32_t m_WindowHeight;
	};
}