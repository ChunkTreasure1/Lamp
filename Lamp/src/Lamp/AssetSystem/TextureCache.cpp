#include "lppch.h"
#include "TextureCache.h"

#include "TextureLoader.h"
#include "Lamp/AssetSystem/AssetManager.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"

namespace Lamp
{
	std::map<std::string, TextureData> TextureCache::m_TextureCache;

	void TextureCache::GetTexture(const std::string& path, Texture2D* pTex)
	{
		LP_PROFILE_FUNCTION();
		auto mit = m_TextureCache.find(path);
		if (mit == m_TextureCache.end())
		{
			g_pEnv->pAssetManager->LoadTexture(path, pTex);
			return;
		}

		pTex->SetData(m_TextureCache[path]);
	}

	void TextureCache::AddTexture(const std::string& path, TextureData data)
	{
		m_TextureCache.emplace(std::pair<std::string, TextureData>(path, data));
	}
}