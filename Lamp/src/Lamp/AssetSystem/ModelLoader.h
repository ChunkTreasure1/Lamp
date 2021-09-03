#pragma once

#include "Lamp/Meshes/Model.h"

namespace Lamp
{
	class ModelLoader
	{
	public:
		static void LoadFromFile(ModelLoadData& data, const std::string& path);
		static Ref<Model> ImportModel(const std::string& path);
		static bool SaveToPath(Ref<Model>& model, const std::string & path);

		static ModelData GenerateMesh(ModelLoadData& data);

	private:
		static std::vector<Ref<Mesh>> LoadModel(const std::string& path);
		static void ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Ref<Mesh>>& meshes);
		static Ref<Mesh> ProcessMesh(aiMesh* pMesh, const aiScene* pScene);
	};
}