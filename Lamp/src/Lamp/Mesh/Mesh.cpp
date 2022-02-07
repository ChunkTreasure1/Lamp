#include "lppch.h"
#include "Mesh.h"

#include "Lamp/Rendering/Renderer.h"
#include "glm/glm.hpp"

#include "Lamp/Math/Math.h"

namespace Lamp
{
	Mesh::Mesh(const std::string& name, std::vector<Ref<SubMesh>> meshes, std::map<uint32_t, Ref<Material>> mats, AABB bb)
		: m_materials(mats), m_name(name), m_boundingBox(bb), m_meshes(meshes)
	{
	}

	Mesh::Mesh()
	{
	}

	void Mesh::Render(size_t id, const glm::mat4& transform)
	{
		for (const auto& mesh : m_meshes)
		{
			//TODO: Render error mesh if mesh is invalid
			Renderer::SubmitMesh(transform, mesh, m_materials[mesh->GetMaterialIndex()], id);
		}
	}

	void Mesh::Load(const std::string& name, std::vector<Ref<SubMesh>> meshes, std::map<uint32_t, Ref<Material>> mats, AABB bb)
	{
		m_name = name;
		m_meshes = meshes;
		m_materials = mats;
		m_boundingBox = bb;
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

	Ref<Mesh> Mesh::Create(const std::string& name, std::vector<Ref<SubMesh>> meshes, std::map<uint32_t, Ref<Material>> mats, AABB bb)
	{
		return CreateRef<Mesh>(name, meshes, mats, bb);
	}

	Ref<Mesh> Mesh::Create()
	{
		return CreateRef<Mesh>();
	}
}