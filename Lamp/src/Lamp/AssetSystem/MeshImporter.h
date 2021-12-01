#pragma once

#include "Lamp/Mesh/Mesh.h"

struct aiMesh;
struct aiScene;
struct aiNode;

namespace Lamp
{
	enum class Units
	{
		Centimeters = 0,
		Decimeters = 1,
		Meters = 2
	};

	struct MeshImportSettings
	{
		std::filesystem::path path;
		bool compileStatic = false;
		Units units = Units::Centimeters;
		glm::vec3 upDirection = { 0.f, 1.f, 0.f };
	};

	class MeshImporter
	{
	public:
		static Ref<Mesh> ImportMesh(const MeshImportSettings& settings);

	private:
		static std::vector<Ref<SubMesh>> LoadMesh(const MeshImportSettings& settings, std::map<uint32_t, Ref<Material>>& materials);
		static void ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Ref<SubMesh>>& meshes);
		static Ref<SubMesh> ProcessMesh(aiMesh* pMesh, const aiScene* pScene);

		static std::vector<Ref<SubMesh>> CompileStatic(const std::vector<Ref<SubMesh>>& meshes, const std::map<uint32_t, Ref<Material>>& materials);
	};
}