#include "lppch.h"
#include "GeometrySystem.h"

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>

namespace Lamp
{
	Model GeometrySystem::ImportModel(const std::string& path)
	{
		std::vector<Mesh> meshes = LoadModel(path);
		Material mat(0);

		std::string t = path;
		t = t.substr(t.find_last_of('/') + 1, t.find_last_of('.'));
		t = t.substr(0, t.find_last_of('.'));


		Model model(meshes, mat, t);

		return model;
	}

	Model GeometrySystem::LoadFromFile(const std::string& path)
	{
		rapidxml::xml_document<> file;
		rapidxml::xml_node<>* pRootNode;

		std::ifstream modelFile(path);
		std::vector<char> buffer((std::istreambuf_iterator<char>(modelFile)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');
	
		file.parse<0>(&buffer[0]);
		pRootNode = file.first_node("Geometry");

		std::string name = "";
		std::vector<Mesh> meshes;
		Material mat(0);

		name = pRootNode->first_attribute("name")->value();

		if (rapidxml::xml_node<>* pMeshes = pRootNode->first_node("Meshes"))
		{
			for (rapidxml::xml_node<>* pMesh = pMeshes->first_node("Mesh"); pMesh; pMesh = pMesh->next_sibling())
			{
				std::vector<Vertex> vertices;
				std::vector<uint32_t> indices;
				
				int matId;
				GetValue(pMesh->first_attribute("matId")->value(), matId);

				if (rapidxml::xml_node<>* pVertices = pMesh->first_node("Vertices"))
				{
					for (rapidxml::xml_node<>* pVertex = pVertices->first_node("Vertex"); pVertex; pVertex->next_sibling())
					{
						glm::vec3 pos(0.f, 0.f, 0.f);
						GetValue(pVertex->first_attribute("position")->value(), pos);

						glm::vec3 normal(0.f, 0.f, 0.f);
						GetValue(pVertex->first_attribute("normal")->value(), normal);

						glm::vec2 texCoords(0.f, 0.f);
						GetValue(pVertex->first_attribute("textureCoordinate")->value(), texCoords);
					
						vertices.push_back(Vertex(pos, normal, texCoords));
					}
				}
				if (rapidxml::xml_node<>* pIndices = pMesh->first_node("Indices"))
				{
					for (rapidxml::xml_node<>* pIndice = pIndices->first_node("Indice"); pIndice; pIndice->next_sibling())
					{
						int value = 0;
						GetValue(pIndice->first_attribute("value")->value(), value);

						indices.push_back((uint32_t)value);
					}
				}			
			
				meshes.push_back(Mesh(vertices, indices, matId));
			}
		}

		if (rapidxml::xml_node<>* pMaterials = pRootNode->first_node("Materials"))
		{
			for (rapidxml::xml_node<>* pMaterial = pMaterials->first_node("Material"); pMaterial; pMaterial = pMaterial->next_sibling())
			{
				std::string name = pMaterial->first_attribute("name")->value();
				std::string diffPath;
				std::string specPath;

				float shininess;
				std::string vertexPath;
				std::string fragmentPath;

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
					GetValue(pShine->first_attribute("Shininess")->value(), shininess);
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

		return Model(meshes, mat, name);
	}

	bool GeometrySystem::SaveToPath(Model& model, const std::string& path)
	{
		using namespace rapidxml;

		LP_CORE_INFO("Saving model to file...");

		std::ofstream file;
		xml_document<> doc;
		file.open(path);

		xml_node<>* pRoot = doc.allocate_node(node_element, "Geometry");
		pRoot->append_attribute(doc.allocate_attribute("name", model.GetName().c_str()));

		/////Meshes/////
		xml_node<>* pMeshes = doc.allocate_node(node_element, "Meshes");
		for (auto& mesh : model.GetMeshes())
		{
			xml_node<>* pMesh = doc.allocate_node(node_element, "Mesh");
			char* pMatId = doc.allocate_string(std::to_string(mesh.GetMaterialIndex()).c_str());
			pMesh->append_attribute(doc.allocate_attribute("matId", pMatId));

			xml_node<>* pVertices = doc.allocate_node(node_element, "Vertices");
			for (auto& vert : mesh.GetVertices())
			{
				xml_node<>* pVert = doc.allocate_node(node_element, "Vertex");

				char* pPos = doc.allocate_string(ToString(vert.position).c_str());
				pVert->append_attribute(doc.allocate_attribute("position", pPos));

				char* pNormal = doc.allocate_string(ToString(vert.normal).c_str());
				pVert->append_attribute(doc.allocate_attribute("normal", pNormal));

				char* pTex = doc.allocate_string(ToString(vert.textureCoords).c_str());
				pVert->append_attribute(doc.allocate_attribute("textureCoordinate", pTex));

				pVertices->append_node(pVert);
			}
			pMesh->append_node(pVertices);

			xml_node<>* pIndices = doc.allocate_node(node_element, "Indices");
			for (auto& indice : mesh.GetIndices())
			{
				xml_node<>* pIndice = doc.allocate_node(node_element, "Indice");

				char* pValue = doc.allocate_string(ToString((int)indice).c_str());
				pIndice->append_attribute(doc.allocate_attribute("value", pValue));

				pIndices->append_node(pIndice);
			}
			pMesh->append_node(pIndices);
			pMeshes->append_node(pMesh);
		}
		pRoot->append_node(pMeshes);
		////////////////

		////Material////
		xml_node<>* pMaterials = doc.allocate_node(node_element, "Materials");

		xml_node<>* pMaterial = doc.allocate_node(node_element, "Material");
		pMaterial->append_attribute(doc.allocate_attribute("name", model.GetMaterial().GetName().c_str()));

		xml_node<>* pDiff = doc.allocate_node(node_element, "Diffuse");
		pDiff->append_attribute(doc.allocate_attribute("path", model.GetMaterial().GetDiffuse()->GetPath().c_str()));
		pMaterial->append_node(pDiff);

		xml_node<>* pSpec = doc.allocate_node(node_element, "Specular");
		pSpec->append_attribute(doc.allocate_attribute("path", model.GetMaterial().GetSpecular()->GetPath().c_str()));
		pMaterial->append_node(pSpec);

		xml_node<>* pShine = doc.allocate_node(node_element, "Shininess");
		char* pS = doc.allocate_string(ToString(model.GetMaterial().GetShininess()).c_str());
		pShine->append_attribute(doc.allocate_attribute("value", pS));
		pMaterial->append_node(pShine);

		xml_node<>* pShader = doc.allocate_node(node_element, "Shader");
		pShader->append_attribute(doc.allocate_attribute("vertex", model.GetMaterial().GetShader()->GetVertexPath().c_str()));
		pShader->append_attribute(doc.allocate_attribute("fragment", model.GetMaterial().GetShader()->GetFragementPath().c_str()));
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

	std::vector<Mesh> GeometrySystem::LoadModel(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene* pScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
		{
			std::string err = "ERROR::ASSIMP::" + std::string(importer.GetErrorString());
			LP_CORE_ERROR(err);
		}

		std::vector<Mesh> meshes;
		ProcessNode(pScene->mRootNode, pScene, meshes);

		return meshes;
	}

	void GeometrySystem::ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Mesh>& meshes)
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

	Mesh GeometrySystem::ProcessMesh(aiMesh* pMesh, const aiScene* pScene)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		for (size_t i = 0; i < pMesh->mNumVertices; i++)
		{
			Vertex vert;

			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 texCoords;

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

		return Mesh(vertices, indices, pMesh->mMaterialIndex);
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