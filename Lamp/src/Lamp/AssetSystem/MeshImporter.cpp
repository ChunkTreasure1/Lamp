#include "lppch.h"
#include "MeshImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Lamp
{
	static const uint32_t s_MeshImportFlags =
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords |
		aiProcess_OptimizeMeshes |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ValidateDataStructure;

	Ref<Mesh> MeshImporter::ImportMesh(const ImportSettings& settings)
	{
		std::map<uint32_t, Ref<Material>> materials;
		std::vector<Ref<SubMesh>> meshes = LoadMesh(settings, materials);
		if (meshes.empty())
		{
			LP_CORE_WARN("Failed to load mesh {0}!", settings.path.string().c_str());
			Ref<Mesh> asset = CreateRef<Mesh>();
			asset->SetFlag(AssetFlag::Invalid);
			return asset;
		}

		//Create materials

		float xMax = FLT_MIN, yMax = FLT_MIN, zMax = FLT_MIN;
		float xMin = FLT_MAX, yMin = FLT_MAX, zMin = FLT_MAX;

		for (auto& mesh : meshes)
		{
			for (auto& vert : mesh->GetVertices())
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
		}

		AABB boundingBox;
		boundingBox.Max = glm::vec3(xMax, yMax, zMax);
		boundingBox.Min = glm::vec3(xMin, yMin, zMin);

		Ref<Mesh> mesh = CreateRef<Mesh>(settings.path.stem().string(), meshes, materials, boundingBox);

		return mesh;
	}

	std::vector<Ref<SubMesh>> MeshImporter::LoadMesh(const ImportSettings& settings, std::map<uint32_t, Ref<Material>>& materials)
	{
		Assimp::Importer importer;
		std::vector<Ref<SubMesh>> meshes;

		const aiScene* pScene = importer.ReadFile(settings.path.string(), s_MeshImportFlags);
		if (!pScene)
		{
			return meshes;
		}

		if (pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||!pScene->mRootNode)
		{
			std::string err = "[Assimp] " + std::string(importer.GetErrorString());
			LP_CORE_ERROR(err);
			return meshes;
		}

		ProcessNode(pScene->mRootNode, pScene, meshes);

		for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
		{
			materials.emplace(i, CreateRef<Material>(i, pScene->mMaterials[i]->GetName().C_Str()));
		}

		return meshes;
	}

	void MeshImporter::ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Ref<SubMesh>>& meshes)
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
	}

	Ref<SubMesh> MeshImporter::ProcessMesh(aiMesh* pMesh, const aiScene* pScene)
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

			pos.x = pMesh->mVertices[i].x;
			pos.y = pMesh->mVertices[i].y;
			pos.z = pMesh->mVertices[i].z;

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

		return CreateRef<SubMesh>(vertices, indices, pMesh->mMaterialIndex);
	}
}