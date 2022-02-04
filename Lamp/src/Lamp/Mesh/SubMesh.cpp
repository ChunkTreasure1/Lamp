#include "lppch.h"
#include "SubMesh.h"

namespace Lamp
{
	SubMesh::SubMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t matIndex)
		: m_MaterialIndex(matIndex)
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