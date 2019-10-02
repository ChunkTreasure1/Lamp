#include "lppch.h"
#include "TextureCache.h"
#include "ImageLoader.h"

namespace Lamp
{
	TextureCache::TextureCache()
	{}

	TextureCache::~TextureCache()
	{}

	GLTexture TextureCache::GetTexture(std::string texturePath)
	{
		//Find the requested texture
		auto mit = m_TextureMap.find(texturePath);

		//Check if the texture was found
		if (mit == m_TextureMap.end())
		{
			//Load the texture
			GLTexture newTexture = ImageLoader::LoadPNG(texturePath);

			//Insert the texture into the texture map
			std::pair<std::string, GLTexture> newPair(texturePath, newTexture);
			m_TextureMap.insert(newPair);

			LP_CORE_INFO("Loaded texture!");
			return newTexture;
		}
		LP_CORE_INFO("Used chached texture!");
		return mit->second;
	}
}
