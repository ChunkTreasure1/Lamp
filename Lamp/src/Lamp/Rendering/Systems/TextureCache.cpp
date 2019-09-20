#include "TextureCache.h"
#include "ImageLoader.h"

#include <iostream>

namespace CactusEngine
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

			std::cout << "Used cached Texture!\n";
			return newTexture;
		}

		std::cout << "Loaded Texture!\n";
		return mit->second;
	}
}
