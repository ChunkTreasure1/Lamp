#include "lppch.h"
#include "ModelLoader.h"

#include <rapidxml/rapidxml.hpp>
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

	void ModelLoader::LoadFromFile(ModelLoadData& data, const std::string& path)
	{
		struct stat b;
		if (!(stat(path.c_str(), &b) == 0))
		{
			return;
		}

		rapidxml::xml_document<> file;
		rapidxml::xml_node<>* pRootNode;

		std::ifstream modelFile(path + ".spec");
		std::vector<char> buffer((std::istreambuf_iterator<char>(modelFile)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');

		file.parse<0>(&buffer[0]);
		pRootNode = file.first_node("Geometry");

		std::string name = "";
		std::vector<MeshData> meshes;
		Material mat(0);
		SAABB boundingBox;

		name = pRootNode->first_attribute("name")->value();

		if (rapidxml::xml_node<>* pMeshes = pRootNode->first_node("Meshes"))
		{
			for (rapidxml::xml_node<>* pMesh = pMeshes->first_node("Mesh"); pMesh; pMesh = pMesh->next_sibling())
			{
				std::vector<Vertex> vertices;
				std::vector<uint32_t> indices;

				int matId;
				GetValue(pMesh->first_attribute("matId")->value(), matId);

				int vertCount = 0;
				if (rapidxml::xml_node<>* pVCount = pMesh->first_node("VerticeCount"))
				{
					GetValue(pVCount->first_attribute("count")->value(), vertCount);
				}

				int indiceCount = 0;
				if (rapidxml::xml_node<>* pICount = pMesh->first_node("IndiceCount"))
				{
					GetValue(pICount->first_attribute("count")->value(), indiceCount);
				}

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

				meshes.push_back(MeshData{ vertices, indices, (uint32_t)matId });
			}
		}

		if (rapidxml::xml_node<>* pMaterials = pRootNode->first_node("Materials"))
		{
			for (rapidxml::xml_node<>* pMaterial = pMaterials->first_node("Material"); pMaterial; pMaterial = pMaterial->next_sibling())
			{
				std::string name;

				name = pMaterial->first_attribute("name")->value();
				mat = MaterialLibrary::GetMaterial(name);
			}
		}

		if (rapidxml::xml_node<>* pBB = pRootNode->first_node("BoundingBox"))
		{
			if (rapidxml::xml_node<>* pMax = pBB->first_node("Max"))
			{
				GetValue(pMax->first_attribute("position")->value(), boundingBox.StartMax);
			}
			if (rapidxml::xml_node<>* pMin = pBB->first_node("Min"))
			{
				GetValue(pMin->first_attribute("position")->value(), boundingBox.StartMin);
			}

			boundingBox.Max = boundingBox.StartMax;
			boundingBox.Min = boundingBox.StartMin;
		}

		data.boundingBox = boundingBox;
		data.material = mat;
		data.meshes = meshes;
		data.name = name;
		data.path = path;
	}

	ModelData ModelLoader::GenerateMesh(ModelLoadData& data)
	{
		ModelData d;
		d.boundingBox = data.boundingBox;
		d.material = data.material;
		d.name = data.name;
		d.path = data.path;

		for (auto& mesh : data.meshes)
		{
			d.meshes.push_back(CreateRef<Mesh>(mesh.vertices, mesh.indices, mesh.matId));
		}

		return d;
	}
}