#include "lppch.h"
#include "MaterialLibrary.h"
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

#include "Lamp/Input/FileSystem.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/AssetSystem/ResourceCache.h"

namespace Lamp
{
	std::vector<Ref<Material>> MaterialLibrary::m_Materials;

	static std::string ToString(const float& var)
	{
		return std::to_string(var);
	}

	static bool GetValue(char* val, float& var)
	{
		if (val)
		{
			var = (float)atof(val);
			return true;
		}
		return false;
	}

	void MaterialLibrary::AddMaterial(Ref<Material> mat)
	{
		m_Materials.push_back(mat);
	}

	void MaterialLibrary::AddMaterial(const std::filesystem::path& path)
	{
		LoadMaterial(path);
	}

	void MaterialLibrary::LoadMaterials()
	{
		std::vector<std::string> paths;
		FileSystem::GetAllFilesOfType(paths, ".mtl", "assets");

		for (std::string& path : paths)
		{
			LoadMaterial(path);
		}

		paths.clear();
		FileSystem::GetAllFilesOfType(paths, ".mtl", "engine");
		for (std::string& path : paths)
		{
			LoadMaterial(path);
		}
	}

	Ref<Material> MaterialLibrary::GetMaterial(const std::string& name)
	{
		for (auto& mat : m_Materials)
		{
			if (mat->GetName() == name)
			{
				return mat;
			}
		}

		return nullptr;
	}

	bool MaterialLibrary::IsMaterialLoaded(const std::string& name)
	{
		for (auto& mat : m_Materials)
		{
			if (mat->GetName() == name)
			{
				return true;
			}
		}

		return false;
	}

	void MaterialLibrary::LoadMaterial(const std::filesystem::path& path)
	{
		Ref<Asset> mat = CreateRef<Material>();
		g_pEnv->pAssetManager->LoadAsset(path, mat);
		if (!mat->IsValid())
		{
			LP_CORE_WARN("Unable to load asset {0}!", path.string());
			return;
		}

		AddMaterial(std::dynamic_pointer_cast<Material>(mat));
	}
}