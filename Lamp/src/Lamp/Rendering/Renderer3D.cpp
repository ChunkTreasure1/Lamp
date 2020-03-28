#include "lppch.h"
#include "Renderer3D.h"

#include <glad/glad.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Lamp/Core/Application.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"

namespace Lamp
{
	struct Renderer3DStorage
	{
		PerspectiveCamera* pCamera;

		Ref<VertexArray> pSphereArray;
		Ref<VertexArray> pLineVertexArray;
	};

	Ref<FrameBuffer> Renderer3D::m_pFrameBuffer = nullptr;
	static Renderer3DStorage* s_pData;

	void Renderer3D::Initialize()
	{
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);

		s_pData = new Renderer3DStorage();
		CreateSphere(6.f);

		//////////Lines////////
		s_pData->pLineVertexArray = VertexArray::Create();
		Ref<VertexBuffer> pLineVB;

		std::vector<Vertex> lineVerts;
		pLineVB.reset(VertexBuffer::Create(lineVerts, sizeof(Vertex) * lineVerts.size()));
		pLineVB->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" }
			});
		s_pData->pLineVertexArray->AddVertexBuffer(pLineVB);

		std::vector<uint32_t> lineIndices{ 0, 1 };
		Ref<IndexBuffer> pLineIB;
		pLineIB.reset(IndexBuffer::Create(lineIndices, sizeof(lineIndices) / sizeof(uint32_t)));
		s_pData->pLineVertexArray->SetIndexBuffer(pLineIB);

		m_pFrameBuffer = Lamp::FrameBuffer::Create(1280, 720);
	}
	void Renderer3D::Shutdown()
	{
	}
	void Renderer3D::Begin(PerspectiveCamera& camera, bool isMain)
	{
		if (isMain)
		{
			m_pFrameBuffer->Bind();
			Lamp::Renderer::Clear();
		}

		s_pData->pCamera = &camera;
	}

	void Renderer3D::End()
	{
		m_pFrameBuffer->Unbind();
	}

	void Renderer3D::DrawMesh(const glm::mat4& modelMatrix, Mesh& mesh, Material& mat)
	{
		glActiveTexture(GL_TEXTURE0);
		mat.GetDiffuse()->Bind();
		glActiveTexture(GL_TEXTURE1);
		mat.GetSpecular()->Bind();

		mat.GetShader()->Bind();
		mat.GetShader()->UploadMat4("u_Model", modelMatrix);

		mat.GetShader()->UploadMat4("u_ViewProjection", s_pData->pCamera->GetViewProjectionMatrix());
		mat.GetShader()->UploadFloat3("u_CameraPosition", s_pData->pCamera->GetPosition());
		mat.GetShader()->UploadFloat3("u_SpotLight.position", s_pData->pCamera->GetPosition());
		mat.GetShader()->UploadFloat3("u_SpotLight.direction", s_pData->pCamera->GetFront());

		mesh.GetVertexArray()->Bind();
		Renderer::DrawIndexed(mesh.GetVertexArray());
	}

	void Renderer3D::DrawSphere()
	{
		s_pData->pSphereArray->Bind();
		Renderer::DrawIndexed(s_pData->pSphereArray);
	}
	void Renderer3D::DrawLine(const glm::vec3& posA, const glm::vec3& posB)
	{
		std::vector<Vertex> verts;
		Vertex vertA, vertB;
		vertA.position = posA;
		vertB.position = posB;

		verts.push_back(vertA);
		verts.push_back(vertB);

		s_pData->pLineVertexArray->GetVertexBuffer()[0]->SetVertices(verts, sizeof(Vertex) * verts.size());

		s_pData->pLineVertexArray->Bind();
		Renderer::DrawIndexedLines(s_pData->pLineVertexArray);
	}
	void Renderer3D::CreateSphere(float radius)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		float sectorCount = 10.f;
		float stackCount = 10.f;
		float x, y, z, xy;
		float nx, ny, nz, lengthInv = 1.f / radius;
		float s, t;

		float sectorStep = 2 * glm::pi<float>() / sectorCount;
		float stackStep = glm::pi<float>() / stackCount;
		float sectorAngle, stackAngle;

		for (size_t i = 0; i <= stackCount; ++i)
		{
			stackAngle = glm::pi<float>() / 2 - i * stackStep;
			xy = radius * cosf(stackAngle);
			z = radius * sinf(stackAngle);

			for (size_t j = 0; j <= sectorCount; ++j)
			{
				Vertex vert;

				sectorAngle = j * sectorStep;

				x = xy * cosf(sectorAngle);
				y = xy * sinf(sectorAngle);

				vert.position = { x, y, z };

				nx = x * lengthInv;
				ny = y * lengthInv;
				nz = z * lengthInv;

				vert.normal = { nx, ny, nz };

				s = (float)j / sectorCount;
				t = (float)i / stackCount;
				vert.textureCoords = { s, t };

				vertices.push_back(vert);
			}
		}

		int k1, k2;
		for (size_t i = 0; i < stackCount; ++i)
		{
			k1 = i * (sectorCount + 1);
			k2 = k1 + sectorCount + 1;

			for (size_t j = 0; j < sectorCount; ++j, ++k1, ++k2)
			{
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}

				if (i != (stackCount - 1))
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}

		s_pData->pSphereArray = VertexArray::Create();

		Ref<VertexBuffer> pBuffer;
		pBuffer.reset(VertexBuffer::Create(vertices, sizeof(Vertex) * vertices.size()));
		pBuffer->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float2, "a_TexCoords" }
			});

		s_pData->pSphereArray->AddVertexBuffer(pBuffer);

		Ref<IndexBuffer> pIndexBuffer;
		pIndexBuffer.reset(IndexBuffer::Create(indices, indices.size()));
		s_pData->pSphereArray->SetIndexBuffer(pIndexBuffer);

		s_pData->pSphereArray->Unbind();

	}
}