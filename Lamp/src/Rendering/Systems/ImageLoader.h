#pragma once
#include "../Textures/GLTexture.h"

#include <string>

namespace CactusEngine
{
	class ImageLoader
	{
	public:
		static GLTexture LoadPNG(std::string filePath);
	};

}