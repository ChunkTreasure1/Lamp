#pragma once

#include <unordered_map>
#include "Lamp/Meshes/Model.h"

namespace Lamp
{
	class ResourceCache
	{
	public:
		static bool AddModel(const std::string& path, Ref<Model>& model);
		static bool AddTexture(const std::string& path, TextureData& tex);

		static Ref<Model> GetModel(const std::string& path);
		static void GetTexture(const std::string& path, Texture2D* pTex);

	private:
		ResourceCache() = delete;

	private:
		static std::unordered_map<std::string, Ref<Model>> s_ModelCache;
		static std::unordered_map<std::string, TextureData> s_TextureCache;
	};
}