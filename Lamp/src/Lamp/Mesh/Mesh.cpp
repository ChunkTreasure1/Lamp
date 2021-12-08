#include "lppch.h"
#include "Mesh.h"

#include "Lamp/Rendering/Renderer3D.h"
#include "Lamp/Rendering/Renderer.h"
#include "glm/glm.hpp"

#include "Lamp/Math/Math.h"

namespace Lamp
{
	void Mesh::Render(size_t id, const glm::mat4& transform)
	{
		for (const auto& mesh : m_Meshes)
		{
			//TODO: Render error mesh if mesh is invalid
			Renderer::SubmitMesh(transform, mesh, m_Materials[mesh->GetMaterialIndex()], id);
		}
	}
}