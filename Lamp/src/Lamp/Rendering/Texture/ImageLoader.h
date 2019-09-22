#pragma once
#include "Lamp/Rendering/Texture/GLTexture.h"

#include <string>

namespace Lamp
{
	class ImageLoader
	{
	public:
		static GLTexture LoadPNG(std::string filePath);
	};

}