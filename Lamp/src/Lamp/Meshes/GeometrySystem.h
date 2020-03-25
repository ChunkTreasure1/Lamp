#pragma once

#include "Lamp/Meshes/Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Lamp
{
	class GeometrySystem
	{
	public:
		Model ImportModel(const std::string& path);
		Model LoadFromFile(const std::string& path);
		bool SaveToPath(const Model& model, const std::string& path);

	private:
		std::vector<Mesh> LoadModel(const std::string& path);
		void ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Mesh>& meshes);
		Mesh ProcessMesh(aiMesh* pMesh, const aiScene* pScene);

		static bool GetValue(char* val, int& var);
		static bool GetValue(char* val, float& var);
		static bool GetValue(char* val, glm::vec2& var);
		static bool GetValue(char* val, glm::vec3& var);
	};
}