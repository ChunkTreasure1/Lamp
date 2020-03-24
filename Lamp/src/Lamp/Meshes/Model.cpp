#include "lppch.h"
#include "Model.h"

#include "Lamp/Rendering/Renderer3D.h"

namespace Lamp
{
	void Model::Draw()
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Material.GetShader()->Bind();

			m_Material.GetShader()->UploadFloat3("u_DirLight.direction", { -0.2f, -1.f, 0.3f });
			m_Material.GetShader()->UploadFloat3("u_DirLight.ambient", { 0.05f, 0.05f, 0.05f });
			m_Material.GetShader()->UploadFloat3("u_DirLight.diffuse", { 0.4f, 0.4f, 0.4f });
			m_Material.GetShader()->UploadFloat3("u_DirLight.specular", { 1.f, 1.f, 1.f });

			m_Material.GetShader()->UploadFloat3("u_PointLight.position", { -1.f, 0.4f, 2.f });
			m_Material.GetShader()->UploadFloat3("u_PointLight.ambient", { 0.05f, 0.05f, 0.05f });
			m_Material.GetShader()->UploadFloat3("u_PointLight.diffuse", { 0.8f, 0.8f, 0.8f });
			m_Material.GetShader()->UploadFloat3("u_PointLight.specular", { 1.f, 1.f, 1.f });
			m_Material.GetShader()->UploadFloat("u_PointLight.constant", 1.f);
			m_Material.GetShader()->UploadFloat("u_PointLight.linear", 0.09f);
			m_Material.GetShader()->UploadFloat("u_PointLight.quadric", 0.032f);

			m_Material.GetShader()->UploadFloat3("u_SpotLight.ambient", { 0.f, 0.f, 0.f });
			m_Material.GetShader()->UploadFloat3("u_SpotLight.diffuse", { 1.f, 1.f, 1.f });
			m_Material.GetShader()->UploadFloat3("u_SpotLight.specular", { 1.f, 1.f, 1.f });

			m_Material.GetShader()->UploadFloat("u_SpotLight.constant", 1.f);
			m_Material.GetShader()->UploadFloat("u_SpotLight.linear", 0.09f);
			m_Material.GetShader()->UploadFloat("u_SpotLight.quadratic", 0.032f);
			m_Material.GetShader()->UploadFloat("u_SpotLight.cutOff", glm::cos(glm::radians(12.5f)));
			m_Material.GetShader()->UploadFloat("u_SpotLight.outerCutOff", glm::cos(glm::radians(15.f)));
			
			m_Material.GetShader()->UploadInt("u_Material.diffuse", 0);
			m_Material.GetShader()->UploadInt("u_Material.specular", 1);
			m_Material.GetShader()->UploadFloat("u_Material.shininess", 32.0f);

			Renderer3D::DrawMesh(m_ModelMatrix, m_Meshes[i], m_Material);
		}
	}

	void Model::LoadModel(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene* pScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
		{
			std::string err = "ERROR::ASSIMP::" + std::string(importer.GetErrorString());
			LP_CORE_ERROR(err);
			return;
		}

		m_Directory = path.substr(0, path.find_last_of('/'));

		ProcessNode(pScene->mRootNode, pScene);
	}

	void Model::ProcessNode(aiNode* pNode, const aiScene* pScene)
	{
		for (size_t i = 0; i < pNode->mNumMeshes; i++)
		{
			aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(pMesh, pScene));
		}

		for (size_t i = 0; i < pNode->mNumChildren; i++)
		{
			ProcessNode(pNode->mChildren[i], pScene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* pMesh, const aiScene* pScene)
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
}