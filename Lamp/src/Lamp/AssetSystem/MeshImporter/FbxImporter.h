#pragma once

#include "MeshImporter.h"
#include "Lamp/Rendering/Vertex.h"

#include <fbxsdk.h>

namespace Lamp
{
	class SubMesh;
	class FbxImporter : public MeshImporter
	{
	public:
		~FbxImporter() override = default;
		
	protected:
		Ref<Mesh> ImportMeshImpl(const MeshImportSettings& settings) override;

	private:
		void FetchGeometryNodes(FbxNode* node, std::vector<FbxNode*>& outNodes);
		Ref<SubMesh> ProcessMesh(FbxMesh* mesh);
		
		void ReadNormal(FbxMesh* mesh, int32_t ctrlPointIndex, int32_t vertCount, glm::vec3& normal);
		void ReadTangent(FbxMesh* mesh, int32_t ctrlPointIndex, int32_t vertCount, glm::vec3& tangent);
		void ReadBitangent(FbxMesh* mesh, int32_t ctrlPointIndex, int32_t vertCount, glm::vec3& bitangent);
	
		void Optimize(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
		int32_t FindVertex(const Vertex& aVert, const std::vector<Vertex>& aUniques);
	};
}