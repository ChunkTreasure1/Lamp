#pragma once

#include "Lamp/Rendering/Textures/TextureCube.h"

namespace Lamp
{
	class OpenGLTextureCube : public TextureCube
	{
	public:
		OpenGLTextureCube(const std::filesystem::path& path);
		OpenGLTextureCube(uint32_t width, uint32_t height);
		~OpenGLTextureCube();

		void Bind(uint32_t slot = 0) const override;
		void SetData(const void* data, uint32_t size) override;

		const uint32_t GetWidth() const override { return m_faceWidth; }
		const uint32_t GetHeight() const override { return m_faceHeight; }
		const uint32_t GetID() const override { return m_rendererId; }

	private:
		uint32_t m_rendererId;
		uint32_t m_faceWidth, m_faceHeight;
	};
}