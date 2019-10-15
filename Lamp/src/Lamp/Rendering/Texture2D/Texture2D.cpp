#include "lppch.h"
#include "Texture2D.h"

#include <glad/glad.h>

#include "TextureCache.h"

namespace Lamp
{
	Texture2D::Texture2D(const std::string& path)
	{
		auto tex = TextureCache::GetTexture(path);
		m_RendererID = std::get<0>(tex);
		m_Width = std::get<1>(tex);
		m_Height = std::get<2>(tex);
	}

	void Texture2D::Bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}
}