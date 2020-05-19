#include "lppch.h"
#include "Model.h"

#include "Lamp/Rendering/Renderer3D.h"
#include "glm/glm.hpp"

namespace Lamp
{
	void Model::Render()
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Material.UploadData();

			Renderer3D::DrawMesh(m_ModelMatrix, m_Meshes[i], m_Material);
		}
	}
}