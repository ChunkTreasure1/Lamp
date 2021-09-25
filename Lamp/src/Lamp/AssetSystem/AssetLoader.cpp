#include "lppch.h"
#include "AssetLoader.h"

#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Rendering/RenderGraph/RenderGraph.h"

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

#include "yaml-cpp/yaml.h"

#include "Lamp/Meshes/Mesh.h"
#include "Lamp/Meshes/Materials/MaterialLibrary.h"

namespace Lamp
{
	static bool GetValue(char* val, glm::vec2& var)
	{
		if (val)
		{
			float x, y;
			if (sscanf(val, "%f,%f", &x, &y) == 2)
			{
				var = glm::vec2(x, y);
				return true;
			}
		}
		return false;
	}
	static bool GetValue(char* val, glm::vec3& var)
	{
		if (val)
		{
			float x, y, z;
			if (sscanf(val, "%f,%f,%f", &x, &y, &z) == 3)
			{
				var = glm::vec3(x, y, z);
				return true;
			}
		}
		return false;
	}
	static bool GetValue(char* val, int& var)
	{
		if (val)
		{
			var = atoi(val);
			return true;
		}

		return false;
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

	static std::string ToString(const int& var)
	{
		return std::to_string(var);
	}
	static std::string ToString(const float& var)
	{
		return std::to_string(var);
	}
	static std::string ToString(const glm::vec2& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y));
		return str;
	}
	static std::string ToString(const glm::vec3& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y) + "," + std::to_string(var.z));
		return str;
	}

	void MeshLoader::Save(const Ref<Asset>& asset) const
	{
		Ref<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(asset);

		//Save raw data
		std::ofstream out(asset->Path, std::ios::out | std::ios::binary);

		for (auto& subMesh : mesh->GetSubMeshes())
		{
			out.write((char*)subMesh->GetVertices().data(), subMesh->GetVertices().size() * sizeof(Vertex));
			out.write((char*)subMesh->GetIndices().data(), subMesh->GetIndices().size() * sizeof(Vertex));
		}

		out.close();

		//Save spec file
		using namespace rapidxml;

		LP_CORE_INFO("Saving mesh {0}", mesh->GetName());
		
		std::ofstream file;
		file.open(asset->Path.string() + ".spec");

		xml_document<> doc;		
		xml_node<>* pRoot = doc.allocate_node(node_element, "Geometry");
		pRoot->append_attribute(doc.allocate_attribute("name", mesh->GetName().c_str()));

		//Meshes
		xml_node<>* pMeshes = doc.allocate_node(node_element, "Meshes");
		for (auto& subMesh : mesh->GetSubMeshes())
		{
			xml_node<>* pMesh = doc.allocate_node(node_element, "Mesh");
			char* pMatId = doc.allocate_string(std::to_string(subMesh->GetMaterialIndex()).c_str());
			pMesh->append_attribute(doc.allocate_attribute("matId", pMatId));

			xml_node<>* pVertCount = doc.allocate_node(node_element, "VerticeCount");
			char* pVCount = doc.allocate_string(ToString((int)subMesh->GetVertices().size()).c_str());
			pVertCount->append_attribute(doc.allocate_attribute("count", pVCount));
			pMesh->append_node(pVertCount);

			xml_node<>* pIndiceCount = doc.allocate_node(node_element, "IndiceCount");
			char* pICount = doc.allocate_string(ToString((int)subMesh->GetIndices().size()).c_str());
			pIndiceCount->append_attribute(doc.allocate_attribute("count", pICount));
			pMesh->append_node(pIndiceCount);

			pMeshes->append_node(pMesh);
		}
		pRoot->append_node(pMeshes);

		//Materials
		xml_node<>* pMaterials = doc.allocate_node(node_element, "Materials");
		for (auto& mat : mesh->GetMaterials())
		{
			xml_node<>* pMaterial = doc.allocate_node(node_element, "Material");
			pMaterial->append_attribute(doc.allocate_attribute("name", mat.second.GetName().c_str()));
			pMaterial->append_attribute(doc.allocate_attribute("id", std::to_string(mat.first).c_str()));

			pMaterials->append_node(pMaterial);
		}
		pRoot->append_node(pMaterials);

		//Bounding box
		xml_node<>* pBB = doc.allocate_node(node_element, "BoundingBox");
		xml_node<>* pMax = doc.allocate_node(node_element, "Max");
		char* pPos1 = doc.allocate_string(ToString(mesh->GetBoundingBox().Max).c_str());
		pMax->append_attribute(doc.allocate_attribute("position", pPos1));

		xml_node<>* pMin = doc.allocate_node(node_element, "Min");
		char* pPos2 = doc.allocate_string(ToString(mesh->GetBoundingBox().Min).c_str());
		pMin->append_attribute(doc.allocate_attribute("position", pPos2));

		pBB->append_node(pMax);
		pBB->append_node(pMin);

		pRoot->append_node(pBB);
		doc.append_node(pRoot);

		file << doc;
		file.close();
	
		LP_CORE_INFO("Saved model {0}!", mesh->GetName());
	}

	bool MeshLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		using namespace rapidxml;

		asset = CreateRef<Mesh>();

		//Check if file exists
		struct stat b;
		if (!(stat(path.string().c_str(), &b) == 0))
		{
			asset->SetFlag(AssetFlag::Missing);
			return false;
		}

		xml_document<> file;
		xml_node<>* pRootNode;

		std::ifstream specFile(path.string() + ".spec");
		std::vector<char> buffer((std::istreambuf_iterator<char>(specFile)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');
		specFile.close();

		file.parse<0>(buffer.data());
		pRootNode = file.first_node("Geometry");
		if (!pRootNode)
		{
			LP_CORE_ERROR("Invalid spec file at {0}!", path.string().c_str());
			asset->SetFlag(AssetFlag::Invalid);
			return false;
		}

		std::string name = "";
		std::vector<Ref<SubMesh>> meshes;
		std::map<uint32_t, Material> materials;
		AABB boundingBox;

		name = pRootNode->first_attribute("name")->value();
		
		//Read meshes
		if (xml_node<>* pMeshes = pRootNode->first_node("Meshes"))
		{
			for (xml_node<>* pMesh = pMeshes->first_node("Mesh"); pMesh; pMesh = pMesh->next_sibling())
			{
				std::vector<Vertex> vertices;
				std::vector<uint32_t> indices;

				//Read spec data
				int matId;
				GetValue(pMesh->first_attribute("matId")->value(), matId);

				int vertCount = 0;
				if (xml_node<>* pVertCount = pMesh->first_node("VerticeCount"))
				{
					GetValue(pVertCount->first_attribute("count")->value(), vertCount);
				}

				int indiceCount = 0;
				if (xml_node<>* pIndiceCount = pMesh->first_node("IndiceCount"))
				{
					GetValue(pIndiceCount->first_attribute("count")->value(), indiceCount);
				}

				//Read file data
				std::ifstream in(path, std::ios::in | std::ios::binary);
				for (size_t i = 0; i < vertCount; i++)
				{
					Vertex vert;
					in.read((char*)&vert, sizeof(Vertex));

					vertices.push_back(vert);
				}
				for (size_t i = 0; i < indiceCount; i++)
				{
					uint32_t indice;
					in.read((char*)&indice, sizeof(uint32_t));

					indices.push_back(indice);
				}

				in.close();
				meshes.push_back(CreateRef<SubMesh>(vertices, indices, (uint32_t)matId));
			}
		}

		//Read materials
		if (xml_node<>* pMaterials = pRootNode->first_node("Materials"))
		{
			for (xml_node<>* pMaterial = pMaterials->first_node("Material"); pMaterial; pMaterial = pMaterial->next_sibling())
			{
				std::string name;
				name = pMaterial->first_attribute("name")->value();

				int id; 
				GetValue(pMaterial->first_attribute("id")->value(), id);
				if (MaterialLibrary::IsMaterialLoaded(name))
				{
					materials[id] = MaterialLibrary::GetMaterial(name);
				}
				else
				{
					LP_CORE_ERROR("Material {0} not loaded!", name);
				}
			}
		}

		//Read bounding box
		if (xml_node<>* pBB = pRootNode->first_node("BoundingBox"))
		{
			if (xml_node<>* pMax = pBB->first_node("Max"))
			{
				GetValue(pMax->first_attribute("position")->value(), boundingBox.StartMax);
			}
			if (xml_node<>* pMin = pBB->first_node("Min"))
			{
				GetValue(pMin->first_attribute("position")->value(), boundingBox.StartMin);
			}

			boundingBox.Max = boundingBox.StartMax;
			boundingBox.Min = boundingBox.StartMin;
		}
		
		asset = CreateRef<Mesh>(name, meshes, materials, boundingBox);
		asset->Path = path;

		return true;
	}

	bool TextureLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = Texture2D::Create(path);
		asset->Path = path;

		return true;
	}

	bool EnvironmentLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		return false;
	}

	void RenderGraphLoader::Save(const Ref<Asset>& asset) const
	{
		Ref<RenderGraph> graph = std::dynamic_pointer_cast<RenderGraph>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "RenderGraph" << YAML::Value;
		{
			out << YAML::BeginMap;
		}
	}

	bool RenderGraphLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		return false;
	}

	void MaterialLoader::Save(const Ref<Asset>& asset) const
	{
		Ref<Material> mat = std::dynamic_pointer_cast<Material>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Material" << YAML::Value;
		{
			out << YAML::BeginMap;

			LP_SERIALIZE_PROPERTY(name, mat->GetName(), out);

			for (auto& tex : mat->GetTextures())
			{
				out << YAML::BeginMap;
				LP_SERIALIZE_PROPERTY_STRING(tex.first, tex.second->Path.string(), out);
			
				out << YAML::EndMap;
			}

			LP_SERIALIZE_PROPERTY(Shader, mat->GetShader()->GetName(), out);
			LP_SERIALIZE_PROPERTY(Shader, mat->GetShader()->GetPath(), out);

			out << YAML::EndMap;
		}

		out << YAML::EndMap;

		std::ofstream fout(asset->Path);
		fout << out.c_str();
	}

	bool MaterialLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		std::ifstream stream(path);
		if (!stream.is_open())
		{
			return false;
		}

		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node root = YAML::Load(strStream.str());
		YAML::Node materialNode = root["Material"];

		Ref<Material> material = CreateRef<Material>();

		std::string tempName;
		LP_DESERIALIZE_PROPERTY(name, tempName, materialNode, "Material");
		material->SetName(tempName);

		return false;
	}
}