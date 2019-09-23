#include "lppch.h"
#include "ImageLoader.h"
#include "Lamp/Rendering/Texture/picoPNG.h"
#include "Lamp/Input/IOManager.h"

namespace Lamp
{
	GLTexture ImageLoader::LoadPNG(std::string filePath)
	{
		//Texture object
		GLTexture texture = {};

		std::vector<unsigned char> in;
		std::vector<unsigned char> out;
		unsigned long width, height;

		if (!IOManager::ReadFileToBuffer(filePath, in))
		{
			LP_CORE_ERROR("Failed to load PNG file to buffer!");
		}
		int errorCode = decodePNG(out, width, height, &(in[0]), in.size());
		if (errorCode != 0)
		{
			LP_CORE_ERROR("decodePNG failed with error: " + std::to_string(errorCode));
		}

		glGenTextures(1, &(texture.Id));
		glBindTexture(GL_TEXTURE_2D, texture.Id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(out[0]));

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		texture.Width = width;
		texture.Height = height;

		return texture;
	}
}