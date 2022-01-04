#pragma once

#include "Lamp/Rendering/Textures/TextureCube.h"

namespace Lamp
{
	class OpenGLTextureCube : public TextureCube
	{
	public:
		OpenGLTextureCube(const std::filesystem::path& path);
		OpenGLTextureCube(ImageFormat format, uint32_t width, uint32_t height);
		~OpenGLTextureCube();

		void Bind(uint32_t slot = 0) const override;
		void SetData(const void* data, uint32_t size) override;
		void SetData(Ref<Image2D> image, uint32_t face, uint32_t mip) override {}

		void StartDataOverride() override {}
		void FinishDataOverride() override {}

		const uint32_t GetWidth() const override { return m_faceWidth; }
		const uint32_t GetHeight() const override { return m_faceHeight; }
		const uint32_t GetID() const override { return m_rendererId; }
		const uint32_t GetMipLevelCount() const override { return 0; }

	private:
		uint32_t m_rendererId;
		uint32_t m_faceWidth, m_faceHeight;
	};
}