#include "ResourceManager.h"

namespace CactusEngine
{
	TextureCache ResourceManager::m_TextureCache;

	GLTexture ResourceManager::GetTexture(std::string texturePath)
	{
		return m_TextureCache.GetTexture(texturePath);
	}
}
