#pragma once

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Material.h"
#include "Lamp/Event/Event.h"
#include "Lamp/Event/ApplicationEvent.h"

namespace Lamp
{
	class Model
	{
	public:
		Model(const std::string& path)
			: m_ModelMatrix(1.f), m_Material(0)
		{
			LoadModel(path);
		}

		void Draw();

		//Setting
		inline void SetModelMatrix(const glm::mat4& mat) { m_ModelMatrix = mat; }

		//Getting
		inline Material& GetMaterial() { return m_Material; }

	private:
		void LoadModel(const std::string& path);
		void ProcessNode(aiNode* pNode, const aiScene* pScene);
		Mesh ProcessMesh(aiMesh* pMesh, const aiScene* pScene);
		
	private:
		Material m_Material;
		std::vector<Mesh> m_Meshes;
		std::string m_Directory;

		glm::mat4 m_ModelMatrix;
	};
}