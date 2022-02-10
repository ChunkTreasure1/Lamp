#include "lppch.h"
#include "SubMesh.h"

namespace Lamp
{
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

	SubMesh::SubMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t matIndex)
		: m_boundingSphere(CalculateBoundingSphere(vertices)), m_MaterialIndex(matIndex)
	{
		m_Vertices = vertices;
		m_Indices = indices;

		SetupMesh();
	}

	Ref<SubMesh> SubMesh::CreateBox()
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

		return CreateRef<SubMesh>(positions, indices, 0);
	}

	Ref<SubMesh> SubMesh::CreateQuad()
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

		return CreateRef<SubMesh>(quadVertices, quadIndices, 0);
	}

	void SubMesh::SetupMesh()
	{
		m_pVertexArray = VertexArray::Create();

		Ref<VertexBuffer> pBuffer = VertexBuffer::Create(m_Vertices, sizeof(Vertex) * (uint32_t)m_Vertices.size());
		
		m_pVertexArray->AddVertexBuffer(pBuffer);

		Ref<IndexBuffer> pIndexBuffer = IndexBuffer::Create(m_Indices, (uint32_t)m_Indices.size());
		m_pVertexArray->SetIndexBuffer(pIndexBuffer);

		m_pVertexArray->Unbind();
	}
}