#pragma once

#include "Lamp/Rendering/Textures/TextureHDR.h"

namespace Lamp
{
	class OpenGLTextureHDR : public TextureHDR
	{
	public:
		OpenGLTextureHDR(const std::filesystem::path& path);
		OpenGLTextureHDR(uint32_t width, uint32_t height);
		~OpenGLTextureHDR();

		void Bind(uint32_t slot = 0) const override;
		void SetData(const void* data, uint32_t size) override;

		const uint32_t GetWidth() const override { return m_width; }
		const uint32_t GetHeight() const override { return m_height; }
		const uint32_t GetID() const override { return m_rendererId; }
	
	private:
		uint32_t m_width, m_height;
		uint32_t m_rendererId;
		uint32_t m_internalFormat, m_dataFormat;
	};
}