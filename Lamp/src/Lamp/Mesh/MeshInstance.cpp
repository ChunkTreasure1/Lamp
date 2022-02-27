#include "lppch.h"
#include "MeshInstance.h"

#include "Lamp/Mesh/Mesh.h"
#include "Lamp/Mesh/Materials/MaterialInstance.h"

namespace Lamp
{
	MeshInstance::MeshInstance(Ref<Mesh> mesh)
	{
		m_sharedMesh = mesh;

		const auto& materials = mesh->GetMaterials();
		for (auto material : materials)
		{
			m_materials.emplace(material.first, MaterialInstance::Create(material.second));
		}
	}

	void MeshInstance::Render(const glm::mat4& transform, uint32_t id)
	{
		m_sharedMesh->Render(transform, id, m_materials);
	}
	
	Ref<MeshInstance> MeshInstance::Create(Ref<Mesh> mesh)
	{
		return CreateRef<MeshInstance>(mesh);
	}
}