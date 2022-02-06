#include "lppch.h"
#include "Mesh.h"

#include "Lamp/Rendering/Renderer.h"
#include "glm/glm.hpp"

#include "Lamp/Math/Math.h"

namespace Lamp
{
	Mesh::Mesh(const std::string& name, std::vector<Ref<SubMesh>> meshes, std::map<uint32_t, Ref<Material>> mats, AABB bb)
		: m_Materials(mats), m_Name(name), m_BoundingBox(bb), m_Meshes(meshes)
	{
	}

	Mesh::Mesh()
	{
	}

	void Mesh::Render(size_t id, const glm::mat4& transform)
	{
		for (const auto& mesh : m_Meshes)
		{
			//TODO: Render error mesh if mesh is invalid
			Renderer::SubmitMesh(transform, mesh, m_Materials[mesh->GetMaterialIndex()], id);
		}
	}

	void Mesh::SetMaterial(Ref<Material> mat, uint32_t id)
	{
		auto it = m_Materials.find(id);
		LP_CORE_ASSERT(it != m_Materials.end(), "Mesh does not contain the specified material id!");

		it->second = mat;
	}

	Ref<Material> Mesh::GetMaterial(uint32_t id)
	{
		auto it = m_Materials.find(id);
		LP_CORE_ASSERT(it != m_Materials.end(), "Mesh does not contain the specified material id!");

		return it->second;
	}

	Ref<Mesh> Mesh::Create(const std::string& name, std::vector<Ref<SubMesh>> meshes, std::map<uint32_t, Ref<Material>> mats, AABB bb)
	{
		return CreateRef<Mesh>(name, meshes, mats, bb);
	}

	Ref<Mesh> Mesh::Create()
	{
		return CreateRef<Mesh>();
	}
}