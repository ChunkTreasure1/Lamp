#include "lppch.h"
#include "ResourceManager.h"

namespace Lamp
{
	TextureCache ResourceManager::m_TextureCache;

	GLTexture ResourceManager::GetTexture(std::string texturePath)
	{
		return m_TextureCache.GetTexture(texturePath);
	}
}
