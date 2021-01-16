#include "lppch.h"
#include "TextureLoader.h"

#include <glad/glad.h>
#include <stb/stb_image.h>

namespace Lamp
{
	std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t> TextureLoader::LoadTexture(const std::string& path)
	{
		uint32_t texture;
		std::string fileEnd = path.substr(path.find_last_of("."), path.size() - 1);

		//Generate texture and bind it to GL_TEXTURE_2D
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		//if (fileEnd.find("hdr"))
		//{
		//	//Set texture wrapping
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//	//Set filtering
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//}
		//else
		{
			//Set texture wrapping
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			//Set filtering
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

		GLenum dataFormat = 0, internalFormat = 0, type = 0;

		//if (fileEnd.find("hdr"))
		//{
		//	internalFormat = GL_RGB16F;
		//	dataFormat = GL_RGB;
		//	type = GL_FLOAT;
		//}
		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
			type = GL_UNSIGNED_BYTE;
		}
		else if (channels == 1)
		{
			internalFormat = GL_R8;
			dataFormat = GL_RED;
			type = GL_UNSIGNED_BYTE;
		}

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, type, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			LP_CORE_WARN("Failed to load texture!");
		}
		stbi_image_free(data);

		return std::tuple(texture, width, height, internalFormat, dataFormat);
	}
}