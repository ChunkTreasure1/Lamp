#pragma once

#include <map>
#include <string>

#include "../Textures/GLTexture.h"

namespace CactusEngine
{
	class TextureCache
	{
	public:
		TextureCache();
		~TextureCache();

		//Functions
		GLTexture GetTexture(std::string texturePath);

	private:

		//Member vars
		std::map<std::string, GLTexture> m_TextureMap;
	};

}
