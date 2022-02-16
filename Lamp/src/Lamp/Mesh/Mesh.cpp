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

	void Mesh::Render(size_t id, const glm::mat4& transform)
	{
		for (const auto& mesh : m_subMeshes)
		{
			//TODO: Render error mesh if mesh is invalid
			RenderCommand::SubmitMesh(transform, mesh, m_materials[mesh->GetMaterialIndex()], id);
		}
	}

	void Mesh::SetMaterial(Ref<Material> mat, uint32_t id)
	{
		auto it = m_materials.find(id);
		LP_CORE_ASSERT(it != m_materials.end(), "Mesh does not contain the specified material id!");

		it->second = mat;
	}

	Ref<Material> Mesh::GetMaterial(uint32_t id)
	{
		auto it = m_materials.find(id);
		LP_CORE_ASSERT(it != m_materials.end(), "Mesh does not contain the specified material id!");

		return it->second;
	}
}