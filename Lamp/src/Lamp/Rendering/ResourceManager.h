#pragma once

#include "Lamp/Rendering/Texture/TextureCache.h"

namespace Lamp
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