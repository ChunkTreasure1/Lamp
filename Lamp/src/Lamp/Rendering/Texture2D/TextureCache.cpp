#include "lppch.h"
#include "TextureCache.h"

#include "TextureLoader.h"

namespace Lamp
{
	std::map<std::string, std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>> TextureCache::m_TextureCache;

	const std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t> TextureCache::GetTexture(const std::string& path)
	{
		auto mit = m_TextureCache.find(path);
		if (mit == m_TextureCache.end())
		{
			auto tex = TextureLoader::LoadTexture(path);
			m_TextureCache.insert(std::make_pair(path, tex));

			return tex;
		}

		return mit->second;
	}
}