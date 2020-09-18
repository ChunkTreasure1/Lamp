#include "lppch.h"
#include "GeometrySystem.h"

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

namespace Lamp
{
	Ref<Model> GeometrySystem::ImportModel(const std::string& path)
	{
		std::vector<Ref<Mesh>> meshes = LoadModel(path);
		Material mat(0);
		SAABB boundingBox;

		std::string t = path;
		t = t.substr(t.find_last_of('/') + 1, t.find_last_of('.'));
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

		Ref<Model> model = std::make_shared<Model>(meshes, mat, t, boundingBox);

		return model;
	}

	Ref<Model> GeometrySystem::LoadFromFile(const std::string& path)
	{
		rapidxml::xml_document<> file;
		rapidxml::xml_node<>* pRootNode;

		std::ifstream modelFile(path + ".spec");
		std::vector<char> buffer((std::istreambuf_iterator<char>(modelFile)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');
	
		file.parse<0>(&buffer[0]);
		pRootNode = file.first_node("Geometry");

		std::string name = "";
		std::vector<Ref<Mesh>> meshes;
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

				meshes.push_back(std::make_shared<Mesh>(vertices, indices, matId));
			}
		}

		if (rapidxml::xml_node<>* pMaterials = pRootNode->first_node("Materials"))
		{
			for (rapidxml::xml_node<>* pMaterial = pMaterials->first_node("Material"); pMaterial; pMaterial = pMaterial->next_sibling())
			{
				std::string name;
				std::string diffPath;
				std::string specPath;

				float shininess = 0;
				std::string vertexPath;
				std::string fragmentPath;

				name = pMaterial->first_attribute("name")->value();

				if (rapidxml::xml_node<>* pDiff = pMaterial->first_node("Diffuse"))
				{
					diffPath = pDiff->first_attribute("path")->value();
				}
				if (rapidxml::xml_node<>* pSpec = pMaterial->first_node("Specular"))
				{
					specPath = pSpec->first_attribute("path")->value();
				}
				if (rapidxml::xml_node<>* pShine = pMaterial->first_node("Shininess"))
				{
					GetValue(pShine->first_attribute("value")->value(), shininess);
				}
				if (rapidxml::xml_node<>* pShader = pMaterial->first_node("Shader"))
				{
					vertexPath = pShader->first_attribute("vertex")->value();
					fragmentPath = pShader->first_attribute("fragment")->value();
				}

				mat.SetDiffuse(Texture2D::Create(diffPath));
				mat.SetSpecular(Texture2D::Create(specPath));
				mat.SetShader(Shader::Create(vertexPath, fragmentPath));
				mat.SetShininess(shininess);
			}
		}

		if (rapidxml::xml_node<>* pBB = pRootNode->first_node("BoundingBox"))
		{
			if (rapidxml::xml_node<>* pMax = pBB->first_node("Max"))
			{
				GetValue(pMax->first_attribute("position")->value(), boundingBox.Max);
			}
			if (rapidxml::xml_node<>* pMin = pBB->first_node("Min"))
			{
				GetValue(pMin->first_attribute("position")->value(), boundingBox.Min);
			}
		}

		return std::make_shared<Model>(meshes, mat, name, boundingBox, path);
	}

	bool GeometrySystem::SaveToPath(Ref<Model>& model, const std::string& path)
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

		xml_node<>* pDiff = doc.allocate_node(node_element, "Diffuse");
		pDiff->append_attribute(doc.allocate_attribute("path", model->GetMaterial().GetDiffuse()->GetPath().c_str()));
		pMaterial->append_node(pDiff);

		xml_node<>* pSpec = doc.allocate_node(node_element, "Specular");
		pSpec->append_attribute(doc.allocate_attribute("path", model->GetMaterial().GetSpecular()->GetPath().c_str()));
		pMaterial->append_node(pSpec);

		xml_node<>* pShine = doc.allocate_node(node_element, "Shininess");
		char* pS = doc.allocate_string(ToString(model->GetMaterial().GetShininess()).c_str());
		pShine->append_attribute(doc.allocate_attribute("value", pS));
		pMaterial->append_node(pShine);

		xml_node<>* pShader = doc.allocate_node(node_element, "Shader");
		pShader->append_attribute(doc.allocate_attribute("vertex", model->GetMaterial().GetShader()->GetVertexPath().c_str()));
		pShader->append_attribute(doc.allocate_attribute("fragment", model->GetMaterial().GetShader()->GetFragmentPath().c_str()));
		pMaterial->append_node(pShader);
		
		pMaterials->append_node(pMaterial);
		////////////////

		pRoot->append_node(pMaterials);
		doc.append_node(pRoot);

		file << doc;
		file.close();

		LP_CORE_INFO("Saved model!");

		return true;
	}

	std::vector<Ref<Mesh>> GeometrySystem::LoadModel(const std::string& path)
	{
		Assimp::Importer importer;
		std::vector<Ref<Mesh>> meshes;

		const aiScene* pScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
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

	void GeometrySystem::ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Ref<Mesh>>& meshes)
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

	Ref<Mesh> GeometrySystem::ProcessMesh(aiMesh* pMesh, const aiScene* pScene)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		for (size_t i = 0; i < pMesh->mNumVertices; i++)
		{
			Vertex vert;

			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 texCoords;

			//'50' is the downscaling, might need some changes
			pos.x = pMesh->mVertices[i].x / 50;
			pos.y = pMesh->mVertices[i].y / 50;
			pos.z = pMesh->mVertices[i].z / 50;

			normal.x = pMesh->mNormals[i].x;
			normal.y = pMesh->mNormals[i].y;
			normal.z = pMesh->mNormals[i].z;

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

		return std::make_shared<Mesh>(vertices, indices, pMesh->mMaterialIndex);
	}

	bool GeometrySystem::GetValue(char* val, glm::vec2& var)
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
	bool GeometrySystem::GetValue(char* val, glm::vec3& var)
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
	bool GeometrySystem::GetValue(char* val, int& var)
	{
		if (val)
		{
			var = atoi(val);
			return true;
		}

		return false;
	}
	bool GeometrySystem::GetValue(char* val, float& var)
	{
		if (val)
		{
			var = (float)atof(val);
			return true;
		}
		return false;
	}

	std::string GeometrySystem::ToString(const int& var)
	{
		return std::to_string(var);
	}
	std::string GeometrySystem::ToString(const float& var)
	{
		return std::to_string(var);
	}
	std::string GeometrySystem::ToString(const glm::vec2& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y));
		return str;
	}
	std::string GeometrySystem::ToString(const glm::vec3& var)
	{
		std::string str(std::to_string(var.x) + "," + std::to_string(var.y) + "," + std::to_string(var.z));
		return str;
	}
}