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
		static Ref<Model> ImportModel(const std::string& path);
		static Ref<Model> LoadFromFile(const std::string& path);
		static bool SaveToPath(Ref<Model>& model, const std::string& path);

	private:
		static std::vector<Mesh> LoadModel(const std::string& path);
		static void ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Mesh>& meshes);
		static Mesh ProcessMesh(aiMesh* pMesh, const aiScene* pScene);

		static bool GetValue(char* val, int& var);
		static bool GetValue(char* val, float& var);
		static bool GetValue(char* val, glm::vec2& var);
		static bool GetValue(char* val, glm::vec3& var);

		static std::string ToString(const int& var);
		static std::string ToString(const float& var);
		static std::string ToString(const glm::vec2& var);
		static std::string ToString(const glm::vec3& var);
	};
}