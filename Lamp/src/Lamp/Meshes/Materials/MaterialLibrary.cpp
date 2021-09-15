#include "lppch.h"
#include "MaterialLibrary.h"
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

#include "Lamp/Input/FileSystem.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/AssetSystem/ResourceCache.h"

namespace Lamp
{
	std::vector<Material> MaterialLibrary::m_Materials;

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

	void MaterialLibrary::AddMaterial(const Material& mat)
	{
		m_Materials.push_back(mat);
	}

	void MaterialLibrary::SaveMaterial(const std::string& path, Material& mat)
	{
		using namespace rapidxml;

		std::ofstream file;
		xml_document<> doc;
		file.open(path);

		xml_node<>* pRoot = doc.allocate_node(node_element, "Material");
		pRoot->append_attribute(doc.allocate_attribute("name", mat.GetName().c_str()));

		for (auto& tex : mat.GetTextures())
		{
			static std::string texPath = "";
			texPath = tex.second ? tex.second->Path.string() : "engine/textures/default/defaultTexture.png";

			xml_node<>* pTex = doc.allocate_node(node_element, tex.first.c_str());
			pTex->append_attribute(doc.allocate_attribute("path", texPath.c_str()));
			pRoot->append_node(pTex);
		}

		xml_node<>* pShader = doc.allocate_node(node_element, "Shader");
		pShader->append_attribute(doc.allocate_attribute("name", mat.GetShader()->GetName().c_str()));
		pShader->append_attribute(doc.allocate_attribute("path", mat.GetShader()->GetPath().c_str()));
		pRoot->append_node(pShader);

		doc.append_node(pRoot);
		file << doc;
		file.close();
	}

	void MaterialLibrary::LoadMaterials()
	{
		using namespace rapidxml;
		std::vector<std::string> paths;
		FileSystem::GetAllFilesOfType(paths, ".mtl", "assets");

		for (std::string& path : paths)
		{
			xml_document<> file;
			xml_node<>* pRoot;

			std::ifstream matFile(path);
			std::vector<char> buffer((std::istreambuf_iterator<char>(matFile)), std::istreambuf_iterator<char>());
			buffer.push_back('\0');

			file.parse<0>(&buffer[0]);
			pRoot = file.first_node("Material");

			std::string shaderName;
			std::string name = pRoot->first_attribute("name")->value();

			if (xml_node<>* pShader = pRoot->first_node("Shader"))
			{
				shaderName = pShader->first_attribute("name")->value();
			}

			Material mat(ShaderLibrary::GetShader(shaderName), 0);
			mat.SetName(name);

			for (auto& texName : mat.GetShader()->GetSpecifications().TextureNames)
			{
				if (xml_node<>* pTex = pRoot->first_node(texName.c_str()))
				{
					mat.SetTexture(texName, ResourceCache::GetAsset<Texture2D>(pTex->first_attribute("path")->value()));
				}
			}

			mat.SetPath(std::filesystem::path(path));
			AddMaterial(mat);
		}
	}

	Material& MaterialLibrary::GetMaterial(const std::string& name)
	{
		for (auto& mat : m_Materials)
		{
			if (mat.GetName() == name)
			{
				return mat;
			}
		}

		return Material(-1);
	}

	bool MaterialLibrary::IsMaterialLoaded(const std::string& name)
	{
		for (auto& mat : m_Materials)
		{
			if (mat.GetName() == name)
			{
				return true;
			}
		}

		return false;
	}
}