#pragma once
#include "../Textures/GLTexture.h"

#include <string>

namespace Lamp
{
	class ImageLoader
	{
	public:
		static GLTexture LoadPNG(std::string filePath);
	};

}