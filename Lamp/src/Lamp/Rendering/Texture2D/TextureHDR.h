#pragma once

#include <string>

namespace Lamp
{
	class TextureHDR
	{
	public:
		TextureHDR(const std::string& path);

	private:
		uint32_t m_FBO;
		uint32_t m_RBO;

		uint32_t m_Width, m_Height;
		uint32_t m_HdrMapId, m_CubeMapId;
		GLenum m_InternalFormat, m_DataFormat;
	};
}