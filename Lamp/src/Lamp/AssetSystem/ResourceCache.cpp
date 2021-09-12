#include "lppch.h"
#include "ResourceCache.h"

#include "AssetManager.h"

namespace Lamp
{
	std::unordered_map<std::string, Ref<Model>> ResourceCache::s_ModelCache;
	std::unordered_map<std::string, TextureData> ResourceCache::s_TextureCache;

	bool ResourceCache::AddModel(const std::string& path, Ref<Model>& model)
	{
		if (s_ModelCache.find(path) == s_ModelCache.end())
		{
			s_ModelCache.insert(std::make_pair(path, model));
			return true;
		}
		return false;
	}

	bool ResourceCache::AddTexture(const std::string& path, TextureData& texture)
	{
		if (s_TextureCache.find(path) == s_TextureCache.end())
		{
			s_TextureCache.insert(std::make_pair(path, texture));
			return true;
		}

		return false;
	}

	Ref<Model> ResourceCache::GetModel(const std::string& path)
	{
		Ref<Model> model = CreateRef<Model>();

		if (s_ModelCache.find(path) != s_ModelCache.end())
		{
			model->m_Meshes = s_ModelCache[path]->m_Meshes;
			model->m_Material = s_ModelCache[path]->m_Material;
			model->m_LGFPath = s_ModelCache[path]->m_LGFPath;
			model->m_BoundingBox = s_ModelCache[path]->m_BoundingBox;
			model->m_ModelMatrix = s_ModelCache[path]->m_ModelMatrix;
			model->m_Name = s_ModelCache[path]->m_Name;

			return model;
		}

		g_pEnv->pAssetManager->LoadModel(path, model.get());
		AddModel(path, model);
	
		return model;
	}

	void ResourceCache::GetTexture(const std::string& path, Texture2D* pTex)
	{
		if (s_TextureCache.find(path) != s_TextureCache.end())
		{
			pTex->SetData(s_TextureCache[path]);
			return;
		}

		g_pEnv->pAssetManager->LoadTexture(path, pTex);
	}
}