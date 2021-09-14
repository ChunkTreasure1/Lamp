#pragma once

#include "Lamp/Meshes/Mesh.h"

namespace Lamp
{
	class MeshImporter
	{
	public:
		static Ref<Mesh> ImportMesh(const std::filesystem::path& path);

	private:
		static std::vector<Ref<SubMesh>> LoadMesh(const std::filesystem::path& path);
		static void ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Ref<SubMesh>>& meshes);
		static Ref<SubMesh> ProcessMesh(aiMesh* pMesh, const aiScene* pScene);
	};
}