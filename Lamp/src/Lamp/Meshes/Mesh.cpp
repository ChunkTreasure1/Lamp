#include "lppch.h"
#include "Mesh.h"

namespace Lamp
{
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, uint32_t matIndex)
		: m_MaterialIndex(matIndex)
	{
		m_Vertices = vertices;
		m_Indices = indices;

		SetupMesh();
	}

	void Mesh::SetupMesh()
	{
		m_pVertexArray = VertexArray::Create();

		Ref<VertexBuffer> pBuffer;
		pBuffer.reset(VertexBuffer::Create(m_Vertices, sizeof(Vertex) * m_Vertices.size()));
		pBuffer->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float2, "a_TexCoords" }
		});

		m_pVertexArray->AddVertexBuffer(pBuffer);

		Ref<IndexBuffer> pIndexBuffer;
		pIndexBuffer.reset(IndexBuffer::Create(m_Indices, m_Indices.size()));
		m_pVertexArray->SetIndexBuffer(pIndexBuffer);

		m_pVertexArray->Unbind();
	}
}