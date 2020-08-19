#pragma once

#include "Lamp/Rendering/Texture2D/Texture2D.h"

namespace Lamp
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(uint32_t width, uint32_t height);

		virtual void Bind(uint32_t slot = 0) const override;

		virtual void SetData(void* data, uint32_t size) override;

		//Getting
		virtual const uint32_t GetWidth() const override { return m_Width; }
		virtual const uint32_t GetHeight() const override { return m_Height; }
		virtual const uint32_t GetID() const override { return m_RendererID; }

	private:
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};
}