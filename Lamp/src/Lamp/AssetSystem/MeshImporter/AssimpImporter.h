#pragma once

#include "Lamp/Mesh/Mesh.h"

#include "MeshImporter.h"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace Lamp
{
	class AssimpImporter : public MeshImporter
	{
	public:
		~AssimpImporter() override = default;
		
	protected:
		Ref<Mesh> ImportMeshImpl(const MeshImportSettings& settings) override;

	private:
		static std::vector<Ref<SubMesh>> LoadMesh(const MeshImportSettings& settings, std::map<uint32_t, Ref<Material>>& materials);
		static void ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Ref<SubMesh>>& meshes);
		static Ref<SubMesh> ProcessMesh(aiMesh* pMesh, const aiScene* pScene);

		static std::vector<Ref<SubMesh>> CompileStatic(const std::vector<Ref<SubMesh>>& meshes, const std::map<uint32_t, Ref<Material>>& materials);
	};
}