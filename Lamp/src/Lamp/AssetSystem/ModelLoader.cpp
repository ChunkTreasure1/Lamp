#include "lppch.h"
#include "ModelLoader.h"

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

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

	Ref<Model> ModelLoader::ImportModel(const std::string& path)
	{
		std::vector<Ref<Mesh>> meshes = LoadModel(path);
		Material mat(0);
		SAABB boundingBox;

		std::string t = path;
		t = t.substr(t.find_last_of('\\') + 1, t.find_last_of('.'));
		t = t.substr(0, t.find_last_of('.'));

		float xMax = FLT_MIN, yMax = FLT_MIN, zMax = FLT_MIN;
		float xMin = FLT_MAX, yMin = FLT_MAX, zMin = FLT_MAX;

		for (auto& vert : meshes[0]->GetVertices())
		{
			//X-axis
			if (vert.position.x < xMin)
			{
				xMin = vert.position.x;
			}
			if (vert.position.x > xMax)
			{
				xMax = vert.position.x;
			}

			//Y-axis
			if (vert.position.y < yMin)
			{
				yMin = vert.position.y;
			}
			if (vert.position.y > yMax)
			{
				yMax = vert.position.y;
			}

			//Z-axis
			if (vert.position.z < zMin)
			{
				zMin = vert.position.z;
			}
			if (vert.position.z > zMax)
			{
				zMax = vert.position.z;
			}
		}

		boundingBox.Max = glm::vec3(xMax, yMax, zMax);
		boundingBox.Min = glm::vec3(xMin, yMin, zMin);

		Ref<Model> model = CreateRef<Model>(meshes, mat, t, boundingBox);

		return model;
	}

	bool ModelLoader::SaveToPath(Ref<Model>& model, const std::string& path)
	{
		std::ofstream out(path, std::ios::out | std::ios::binary);
		out.write((char*)&model->GetMeshes()[0]->GetVertices()[0], model->GetMeshes()[0]->GetVertices().size() * sizeof(Vertex));
		out.write((char*)&model->GetMeshes()[0]->GetIndices()[0], model->GetMeshes()[0]->GetIndices().size() * sizeof(uint32_t));
		out.close();

		using namespace rapidxml;

		LP_CORE_INFO("Saving model to file...");
		model->SetLGFPath(path);

		std::ofstream file;
		xml_document<> doc;
		file.open(path + ".spec");

		xml_node<>* pRoot = doc.allocate_node(node_element, "Geometry");
		pRoot->append_attribute(doc.allocate_attribute("name", model->GetName().c_str()));

		/////Meshes/////
		xml_node<>* pMeshes = doc.allocate_node(node_element, "Meshes");
		for (auto& mesh : model->GetMeshes())
		{
			xml_node<>* pMesh = doc.allocate_node(node_element, "Mesh");
			char* pMatId = doc.allocate_string(std::to_string(mesh->GetMaterialIndex()).c_str());
			pMesh->append_attribute(doc.allocate_attribute("matId", pMatId));

			xml_node<>* pVertCount = doc.allocate_node(node_element, "VerticeCount");
			char* pVCount = doc.allocate_string(ToString((int)mesh->GetVertices().size()).c_str());
			pVertCount->append_attribute(doc.allocate_attribute("count", pVCount));
			pMesh->append_node(pVertCount);

			xml_node<>* pIndiceCount = doc.allocate_node(node_element, "IndiceCount");
			char* pICount = doc.allocate_string(ToString((int)mesh->GetIndices().size()).c_str());
			pIndiceCount->append_attribute(doc.allocate_attribute("count", pICount));
			pMesh->append_node(pIndiceCount);

			pMeshes->append_node(pMesh);
		}
		pRoot->append_node(pMeshes);
		////////////////

		////Material////
		xml_node<>* pMaterials = doc.allocate_node(node_element, "Materials");

		xml_node<>* pMaterial = doc.allocate_node(node_element, "Material");
		pMaterial->append_attribute(doc.allocate_attribute("name", model->GetMaterial().GetName().c_str()));

		pMaterials->append_node(pMaterial);
		////////////////

		/////Bounding box//////
		xml_node<>* pBB = doc.allocate_node(node_element, "BoundingBox");

		xml_node<>* pMax = doc.allocate_node(node_element, "Max");
		char* pPos1 = doc.allocate_string(ToString(model->GetBoundingBox().Max).c_str());
		pMax->append_attribute(doc.allocate_attribute("position", pPos1));

		xml_node<>* pMin = doc.allocate_node(node_element, "Min");
		char* pPos2 = doc.allocate_string(ToString(model->GetBoundingBox().Min).c_str());
		pMin->append_attribute(doc.allocate_attribute("position", pPos2));

		pBB->append_node(pMax);
		pBB->append_node(pMin);
		///////////////////////

		pRoot->append_node(pMaterials);
		pRoot->append_node(pBB);

		doc.append_node(pRoot);

		file << doc;
		file.close();

		LP_CORE_INFO("Saved model!");

		return true;
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

	std::vector<Ref<Mesh>> ModelLoader::LoadModel(const std::string& path)
	{
		Assimp::Importer importer;
		std::vector<Ref<Mesh>> meshes;

		const aiScene* pScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
		if (!pScene)
		{
			return meshes;
		}

		if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
		{
			std::string err = "ERROR::ASSIMP::" + std::string(importer.GetErrorString());
			LP_CORE_ERROR(err);
		}

		ProcessNode(pScene->mRootNode, pScene, meshes);

		return meshes;
	}
	
	void ModelLoader::ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Ref<Mesh>>& meshes)
	{
		for (size_t i = 0; i < pNode->mNumMeshes; i++)
		{
			aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
			meshes.push_back(ProcessMesh(pMesh, pScene));
		}

		for (size_t i = 0; i < pNode->mNumChildren; i++)
		{
			ProcessNode(pNode->mChildren[i], pScene, meshes);
		}

		return;
	}

	Ref<Mesh> ModelLoader::ProcessMesh(aiMesh* pMesh, const aiScene* pScene)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		for (size_t i = 0; i < pMesh->mNumVertices; i++)
		{
			Vertex vert;

			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec3 tangent;
			glm::vec3 bitangent;
			glm::vec2 texCoords;

			//'50' is the downscaling, might need some changes
			pos.x = pMesh->mVertices[i].x * 0.017f;
			pos.y = pMesh->mVertices[i].y * 0.017f;
			pos.z = pMesh->mVertices[i].z * 0.017f;

			normal.x = pMesh->mNormals[i].x;
			normal.y = pMesh->mNormals[i].y;
			normal.z = pMesh->mNormals[i].z;

			tangent.x = pMesh->mTangents[i].x;
			tangent.y = pMesh->mTangents[i].y;
			tangent.z = pMesh->mTangents[i].z;

			bitangent.x = pMesh->mBitangents[i].x;
			bitangent.y = pMesh->mBitangents[i].y;
			bitangent.z = pMesh->mBitangents[i].z;

			if (pMesh->mTextureCoords[0])
			{
				texCoords.x = pMesh->mTextureCoords[0][i].x;
				texCoords.y = pMesh->mTextureCoords[0][i].y;
			}
			else
			{
				texCoords = glm::vec2(0.f);
			}

			vert.position = pos;
			vert.normal = normal;
			vert.tangent = tangent;
			vert.bitangent = bitangent;
			vert.textureCoords = texCoords;

			vertices.push_back(vert);
		}

		for (size_t i = 0; i < pMesh->mNumFaces; i++)
		{
			aiFace face = pMesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		return CreateRef<Mesh>(vertices, indices, pMesh->mMaterialIndex);
	}
}