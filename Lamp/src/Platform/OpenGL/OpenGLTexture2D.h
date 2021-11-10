#pragma once

#include "Lamp/Rendering/Textures/Texture2D.h"

namespace Lamp
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::filesystem::path& path);
		OpenGLTexture2D(uint32_t width, uint32_t height);
		~OpenGLTexture2D();

		void Bind(uint32_t slot = 0) const override;
		void SetData(const void* data, uint32_t size) override;

		//Getting
		const uint32_t GetWidth() const override { return m_Width; }
		const uint32_t GetHeight() const override { return m_Height; }
		const uint32_t GetID() const override { return m_RendererID; }

	private:
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		uint32_t m_InternalFormat, m_DataFormat;
	};
}