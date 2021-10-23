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

struct ImportSettings
{
    std::filesystem::path path;
    Units units = Units::Centimeters;
    glm::vec3 upDirection = { 0.f, 1.f, 0.f };
};

class MeshImporter
{
public:
    static Ref<Mesh> ImportMesh(const ImportSettings& settings);

private:
    static std::vector<Ref<SubMesh>> LoadMesh(const ImportSettings& settings, std::map<uint32_t, Ref<Material>>& materials);
    static void ProcessNode(aiNode* pNode, const aiScene* pScene, std::vector<Ref<SubMesh>>& meshes, Units units);
    static Ref<SubMesh> ProcessMesh(aiMesh* pMesh, const aiScene* pScene, Units units);
};
}