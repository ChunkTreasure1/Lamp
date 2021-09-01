#include "lppch.h"
#include "Model.h"

#include "Lamp/Rendering/Renderer3D.h"
#include "glm/glm.hpp"

#include "Lamp/Math/Math.h"

namespace Lamp
{
	void Model::Render(size_t id)
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Material.UploadData();

			Renderer3D::DrawMesh(m_ModelMatrix, m_Meshes[i], m_Material, id);
		}
	}

	void Model::RenderBoundingBox()
	{
		glm::vec3 scale;
		glm::vec3 rotation;
		glm::vec3 position;

		Math::DecomposeTransform(m_ModelMatrix, position, rotation, scale);

		Renderer3D::DrawLine(position + m_BoundingBox.Max, position + glm::vec3(m_BoundingBox.Max.x, m_BoundingBox.Max.y, m_BoundingBox.Min.z), 1.f);
		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Max.x, m_BoundingBox.Max.y, m_BoundingBox.Min.z), position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Max.y, m_BoundingBox.Min.z), 1.f);
		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Max.y, m_BoundingBox.Min.z), position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Max.y, m_BoundingBox.Max.z), 1.f);
		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Max.y, m_BoundingBox.Max.z), position + m_BoundingBox.Max, 1.f);

		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Max.x, m_BoundingBox.Min.y, m_BoundingBox.Max.z), position + glm::vec3(m_BoundingBox.Max.x, m_BoundingBox.Min.y, m_BoundingBox.Min.z), 1.f);
		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Max.x, m_BoundingBox.Min.y, m_BoundingBox.Min.z), position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Min.y, m_BoundingBox.Min.z), 1.f);
		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Min.y, m_BoundingBox.Min.z), position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Min.y, m_BoundingBox.Max.z), 1.f);
		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Min.y, m_BoundingBox.Max.z), position + glm::vec3(m_BoundingBox.Max.x, m_BoundingBox.Min.y, m_BoundingBox.Max.z), 1.f);

		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Max.x, m_BoundingBox.Max.y, m_BoundingBox.Max.z), position + glm::vec3(m_BoundingBox.Max.x, m_BoundingBox.Min.y, m_BoundingBox.Max.z), 1.f);
		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Max.y, m_BoundingBox.Max.z), position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Min.y, m_BoundingBox.Max.z), 1.f);
		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Max.x, m_BoundingBox.Max.y, m_BoundingBox.Min.z), position + glm::vec3(m_BoundingBox.Max.x, m_BoundingBox.Min.y, m_BoundingBox.Min.z), 1.f);
		Renderer3D::DrawLine(position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Max.y, m_BoundingBox.Min.z), position + glm::vec3(m_BoundingBox.Min.x, m_BoundingBox.Min.y, m_BoundingBox.Min.z), 1.f);
	}
}