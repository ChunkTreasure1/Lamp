#include "lppch.h"
#include "Mesh.h"

#include "Lamp/Rendering/RenderCommand.h"
#include "Lamp/Math/Math.h"

namespace Lamp
{
	Mesh::Mesh(const std::string& name, std::vector<Ref<SubMesh>> meshes, std::map<uint32_t, Ref<Material>> mats)
		: m_materials(mats), m_name(name), m_subMeshes(meshes)
	{
	}

	void Mesh::Render(const glm::mat4& transform, size_t id, std::map<uint32_t, Ref<MaterialInstance>>& materials)
	{
		for (const auto& mesh : m_subMeshes)
		{
			//TODO: Render error mesh if mesh is invalid
			RenderCommand::SubmitMesh(transform, mesh, materials[mesh->GetMaterialIndex()], id);
		}
	}
}