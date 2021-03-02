#pragma once

#include "TextureLoader.h"

namespace Lamp
{
	class Texture2D;
	class TextureCache
	{
	public:
		static void GetTexture(const std::string& path, Texture2D* pTex);
		static void AddTexture(const std::string& path, TextureData data);

	private:
		static std::map<std::string, TextureData> m_TextureCache;
	};
}