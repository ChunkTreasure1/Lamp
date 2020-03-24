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
			: m_ModelMatrix(1.f)
		{
			LoadModel(path);
		}

		void OnEvent(Event& e);

		inline std::vector<Material>& GetMaterials() { return m_Materials; }

	private:
		void LoadModel(const std::string& path);
		void ProcessNode(aiNode* pNode, const aiScene* pScene);
		Mesh ProcessMesh(aiMesh* pMesh, const aiScene* pScene);
		
		bool OnRender(AppRenderEvent& e);

	private:
		std::vector<Material> m_Materials;
		std::vector<Mesh> m_Meshes;
		std::string m_Directory;

		glm::mat4 m_ModelMatrix;
	};
}