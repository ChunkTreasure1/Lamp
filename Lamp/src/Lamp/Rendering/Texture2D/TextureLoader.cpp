#include "lppch.h"
#include "TextureLoader.h"

#include <glad/glad.h>
#include <stb/stb_image.h>

namespace Lamp
{
	std::tuple<uint32_t, uint32_t, uint32_t> TextureLoader::LoadTexture(const std::string& path)
	{
		uint32_t texture;

		//Generate texture and bind it to GL_TEXTURE_2D
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		//Set texture wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//Set filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

		GLenum dataFormat = 0, internalFormat = 0;
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			LP_CORE_WARN("Failed to load texture!");
		}
		stbi_image_free(data);

		return std::tuple(texture, width, height);
	}
}