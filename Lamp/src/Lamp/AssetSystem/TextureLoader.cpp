#include "lppch.h"
#include "TextureLoader.h"

#include <glad/glad.h>
#include <stb/stb_image.h>

namespace Lamp
{
	void TextureLoader::LoadTexture(TextureLoadData& data, const std::string& path)
	{
		//LP_PROFILE_FUNCTION();

		std::string fileEnd = path.substr(path.find_last_of("."), path.size() - 1);
		if (fileEnd.find("hdr") != std::string::npos)
		{
			LoadHDRTexture(data, path);
			return;
		}

		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		unsigned char* pData = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (pData)
		{
			data.channels = (uint32_t)channels;
			data.height = (uint32_t)height;
			data.width = (uint32_t)width;
			data.pData = pData;
			data.path = path;
			data.type = TextureType::Texture2D;
		}
	}

	void TextureLoader::LoadHDRTexture(TextureLoadData& data, const std::string& path)
	{
		int width, height, channels;
		float* pData = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
		if (pData)
		{
			data.channels = channels;
			data.height = height;
			data.width = width;
			data.pData = pData;
			data.type = TextureType::HDR;
		}
	}

	TextureData TextureLoader::GenerateTexture(const TextureLoadData& t)
	{
		//Generate texture and bind it to GL_TEXTURE_2D
		uint32_t texture;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		//Set texture wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//Set filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		GLenum dataFormat = 0, internalFormat = 0, type = 0;

		if (t.channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
		else if (t.channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
			type = GL_UNSIGNED_BYTE;
		}
		else if (t.channels == 1)
		{
			internalFormat = GL_R8;
			dataFormat = GL_RED;
			type = GL_UNSIGNED_BYTE;
		}


		if (t.pData)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, t.width, t.height, 0, dataFormat, type, t.pData);
			glGenerateMipmap(GL_TEXTURE_2D);

			stbi_image_free(t.pData);
		}
		else
		{
			LP_CORE_WARN("Failed to load texture!");
		}

		TextureData data;
		data.dataFormat = dataFormat;
		data.internalFormat = internalFormat;
		data.height = t.height;
		data.width = t.width;
		data.rendererId = texture;

		return data;
	}
	TextureData TextureLoader::GenerateHDR(const TextureLoadData& t)
	{
		uint32_t texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		//Set texture wrapping
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//Set filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (t.pData)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, t.width, t.height, 0, GL_RGB, GL_FLOAT, t.pData);
			stbi_image_free(t.pData);
		}

		GLenum dataFormat = 0, internalFormat = 0, type = 0;

		if (t.channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
		else if (t.channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
			type = GL_UNSIGNED_BYTE;
		}
		else if (t.channels == 1)
		{
			internalFormat = GL_R8;
			dataFormat = GL_RED;
			type = GL_UNSIGNED_BYTE;
		}

		TextureData d;
		d.rendererId = texture;
		d.dataFormat = (uint32_t)dataFormat;
		d.internalFormat = (uint32_t)internalFormat;
		d.width = t.width;
		d.height = t.height;

		return d;
	}
}