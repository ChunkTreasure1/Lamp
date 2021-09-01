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

		Ref<VertexBuffer> pBuffer = VertexBuffer::Create(m_Vertices, sizeof(Vertex) * (uint32_t)m_Vertices.size());
		pBuffer->SetBufferLayout
		({
			{ Type::Float3, "a_Position" },
			{ Type::Float3, "a_Normal" },
			{ Type::Float3, "a_Tangent" },
			{ Type::Float3, "a_Bitangent" },
			{ Type::Float2, "a_TexCoords" },
		});

		m_pVertexArray->AddVertexBuffer(pBuffer);

		Ref<IndexBuffer> pIndexBuffer = IndexBuffer::Create(m_Indices, (uint32_t)m_Indices.size());
		m_pVertexArray->SetIndexBuffer(pIndexBuffer);

		m_pVertexArray->Unbind();
	}
}