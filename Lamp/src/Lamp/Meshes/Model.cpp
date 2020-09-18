#include "lppch.h"
#include "Model.h"

#include "Lamp/Rendering/Renderer3D.h"
#include "glm/glm.hpp"
#include "glm/gtx/matrix_decompose.hpp"

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

	void Model::RenderBoundingBox()
	{
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 position;
		glm::vec3 skew;
		glm::vec4 perspective;

		glm::decompose(m_ModelMatrix, scale, rotation, position, skew, perspective);

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