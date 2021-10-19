#include "lppch.h"
#include "Mesh.h"

#include "Lamp/Rendering/Renderer3D.h"
#include "glm/glm.hpp"

#include "Lamp/Math/Math.h"

namespace Lamp
{
	void Mesh::Render(size_t id, const glm::mat4& transform)
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			Renderer3D::SubmitMesh(transform, m_Meshes[i], m_Materials[m_Meshes[i]->GetMaterialIndex()], id);
		}
	}
}