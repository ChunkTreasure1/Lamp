#include "lppch.h"
#include "SubMesh.h"

#include "Lamp/Rendering/Buffers/VertexBuffer.h"

namespace Lamp
{
	enum class BaseMesh
	{
		Cube,
		Quad,
		Sphere
	};

	static std::unordered_map<BaseMesh, Ref<SubMesh>> s_baseMeshes;

	static BoundingSphere CalculateBoundingSphere(const std::vector<Vertex>& vertices)
	{
		glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
		glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

		for (const auto& vert : vertices)
		{
			minAABB.x = std::min(minAABB.x, vert.position.x);
			minAABB.y = std::min(minAABB.y, vert.position.y);
			minAABB.z = std::min(minAABB.z, vert.position.z);

			maxAABB.x = std::max(maxAABB.x, vert.position.x);
			maxAABB.y = std::max(maxAABB.y, vert.position.y);
			maxAABB.z = std::max(maxAABB.z, vert.position.z);
		}

		return BoundingSphere((maxAABB + minAABB) * 0.5f, glm::length(minAABB - maxAABB));
	}

	SubMesh::SubMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t matIndex, const std::string& name)
		: m_boundingSphere(CalculateBoundingSphere(vertices)), m_materialIndex(matIndex), m_name(name)
	{
		m_vertices = vertices;
		m_indices = indices;

		SetupMesh();
	}

	Ref<SubMesh> SubMesh::CreateCube()
	{
		Ref<SubMesh> mesh;

		auto it = s_baseMeshes.find(BaseMesh::Cube);
		if (it == s_baseMeshes.end())
		{
			std::vector<Vertex> positions =
			{
				Vertex({-1, -1, -1}),
				Vertex({ 1, -1, -1}),
				Vertex({ 1,  1, -1}),
				Vertex({-1,  1, -1}),
				Vertex({-1, -1,  1}),
				Vertex({ 1, -1,  1}),
				Vertex({ 1,  1,  1}),
				Vertex({-1,  1,  1}),
			};

			std::vector<uint32_t> indices =
			{
				0, 1, 3,
				3, 1, 2,
				1, 5, 2,
				2, 5, 6,
				5, 4, 6,
				6, 4, 7,
				4, 0, 7,
				7, 0, 3,
				3, 2, 7,
				7, 2, 5,
				4, 5, 0,
				0, 5, 1
			};

			mesh = CreateRef<SubMesh>(positions, indices, 0, "Cube");
			s_baseMeshes[BaseMesh::Cube] = mesh;
		}
		else
		{
			mesh = it->second;
		}

		return mesh;
	}

	Ref<SubMesh> SubMesh::CreateQuad()
	{
		Ref<SubMesh> mesh;

		auto it = s_baseMeshes.find(BaseMesh::Quad);
		if (it == s_baseMeshes.end())
		{
			std::vector<Vertex> quadVertices =
			{
				Vertex({ -1.f, -1.f, 0.f }, { 0.f, 1.f }),
				Vertex({  1.f, -1.f, 0.f }, { 1.f, 1.f }),
				Vertex({  1.f,  1.f, 0.f }, { 1.f, 0.f }),
				Vertex({ -1.f,  1.f, 0.f }, { 0.f, 0.f }),
			};

			std::vector<uint32_t> quadIndices =
			{
				0, 1, 2,
				2, 3, 0
			};

			mesh = CreateRef<SubMesh>(quadVertices, quadIndices, 0, "Quad");
			s_baseMeshes[BaseMesh::Quad] = mesh;
		}
		else
		{
			mesh = it->second;
		}

		return mesh;
	}

	Ref<SubMesh> SubMesh::CreateSphere()
	{
		return nullptr;
	}

	void SubMesh::SetupMesh()
	{
		m_vertexBuffer = VertexBuffer::Create(m_vertices, sizeof(Vertex) * (uint32_t)m_vertices.size());
		m_indexBuffer = IndexBuffer::Create(m_indices, (uint32_t)m_indices.size());
	}
}