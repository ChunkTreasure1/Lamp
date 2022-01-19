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