#include "lppch.h"
#include "MaterialLibrary.h"
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

#include "Lamp/Input/FileSystem.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/AssetSystem/ResourceCache.h"

namespace Lamp
{
	MaterialLibrary* MaterialLibrary::s_instance = nullptr;

	MaterialLibrary::MaterialLibrary()
	{
		LP_CORE_ASSERT(!s_instance, "Instance already created! The should not be more than one instance!");
		s_instance = this;
	}

	void MaterialLibrary::Initialize()
	{

	}

	void MaterialLibrary::Shutdown()
	{
		m_materials.clear();
	}

	void MaterialLibrary::AddMaterial(Ref<Material> mat)
	{
		m_materials.push_back(mat);
	}

	void MaterialLibrary::AddMaterial(const std::filesystem::path& path)
	{
		LoadMaterial(path);
	}

	void MaterialLibrary::LoadMaterials()
	{
		m_materials.clear();

		std::vector<std::string> paths;
		FileSystem::GetAllFilesOfType(".mtl", "assets", paths);

		for (std::string& path : paths)
		{
			LoadMaterial(path);
		}

		paths.clear();
		FileSystem::GetAllFilesOfType(".mtl", "engine", paths);
		for (std::string& path : paths)
		{
			LoadMaterial(path);
		}
	}

	Ref<Material> MaterialLibrary::GetMaterial(const std::string& name)
	{
		for (auto& mat : s_instance->m_materials)
		{
			if (mat->GetName() == name)
			{
				return mat;
			}
		}

		return nullptr;
	}

	MaterialLibrary& MaterialLibrary::Get()
	{
		return *s_instance;
	}

	bool MaterialLibrary::IsMaterialLoaded(const std::string& name)
	{
		for (auto& mat : m_materials)
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
		Ref<Asset> mat = Material::Create();
		g_pEnv->pAssetManager->LoadAsset(path, mat);
		if (!mat->IsValid())
		{
			LP_CORE_WARN("Unable to load asset {0}!", path.string());
			return;
		}

		AddMaterial(std::dynamic_pointer_cast<Material>(mat));
	}
}