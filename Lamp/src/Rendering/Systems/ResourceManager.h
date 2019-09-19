#pragma once

#include "TextureCache.h"

namespace CactusEngine
{
	class ResourceManager
	{
	public:

		//Functions
		static GLTexture GetTexture(std::string texturePath);

	private:

		//Member vars
		static TextureCache m_TextureCache;
	};
}