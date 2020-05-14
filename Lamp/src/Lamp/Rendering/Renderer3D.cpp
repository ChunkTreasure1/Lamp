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
	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
	};

	struct Renderer3DStorage
	{
		static const uint32_t MaxLines = 10000;
		static const uint32_t MaxLineVerts = MaxLines * 2;
		static const uint32_t MaxLineIndices = MaxLines * 2;

		//////Lines//////
		Ref<VertexArray> pLineVertexArray;
		Ref<VertexBuffer> pLineVertexBuffer;
		uint32_t LineIndexCount = 0;

		LineVertex* pLineVertexBufferBase = nullptr;
		LineVertex* pLineVertexBufferPtr = nullptr;
		/////////////////

		Renderer3DStorage()
			: material(Lamp::Texture2D::Create("engine/textures/default/defaultTexture.png"), Lamp::Texture2D::Create("engine/textures/default/defaultTexture.png"), Lamp::Shader::Create("engine/shaders/Texture.vert", "engine/shaders/Texture.frag"), 0)
		{}

		PerspectiveCamera* pCamera;
		Ref<VertexArray> pSphereArray;

		Material material;
	};

	Ref<FrameBuffer> Renderer3D::m_pFrameBuffer = nullptr;
	static Renderer3DStorage* s_pData;

	void Renderer3D::Initialize()
	{
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);

		s_pData = new Renderer3DStorage();

		///////Line///////
		s_pData->pLineVertexArray = VertexArray::Create();
		s_pData->pLineVertexBuffer = VertexBuffer::Create(s_pData->MaxLineVerts * sizeof(LineVertex));
		s_pData->pLineVertexBuffer->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float4, "a_Color" }
			});
		s_pData->pLineVertexArray->AddVertexBuffer(s_pData->pLineVertexBuffer);
		s_pData->pLineVertexBufferBase = new LineVertex[s_pData->MaxLineVerts];

		uint32_t* pLineIndices = new uint32_t[s_pData->MaxLineIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_pData->MaxLineIndices; i += 2)
		{
			pLineIndices[i + 0] = offset + 0;
			pLineIndices[i + 1] = offset + 1;

			offset += 2;
		}

		Ref<IndexBuffer> pLineIB = IndexBuffer::Create(pLineIndices, s_pData->MaxLineIndices);
		s_pData->pLineVertexArray->SetIndexBuffer(pLineIB);

		delete[] pLineIndices;
		//////////////////

		//m_pFrameBuffer = Lamp::FrameBuffer::Create(1280, 720);
	}

	void Renderer3D::Shutdown()
	{
		delete s_pData;
	}

	void Renderer3D::Begin(PerspectiveCamera& camera, bool isMain)
	{
		if (isMain)
		{
			//m_pFrameBuffer->Bind();
			Lamp::Renderer::Clear();
		}

		s_pData->pCamera = &camera;

		ResetBatchData();
	}

	void Renderer3D::End()
	{
		//m_pFrameBuffer->Unbind();

		uint32_t dataSize = (uint8_t*)s_pData->pLineVertexBufferPtr - (uint8_t*)s_pData->pLineVertexBufferBase;
		s_pData->pLineVertexBuffer->SetData(s_pData->pLineVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer3D::Flush()
	{
		s_pData->material.GetShader()->Bind();
		s_pData->material.GetShader()->UploadMat4("u_ViewProjection", s_pData->pCamera->GetViewProjectionMatrix());

		Renderer::DrawIndexedLines(s_pData->pLineVertexArray, s_pData->LineIndexCount);
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

		/////Lighting/////
		glm::mat3 normalMat = glm::transpose(glm::inverse(modelMatrix));
		mat.GetShader()->UploadMat3("u_NormalMatrix", normalMat);

		mat.GetShader()->UploadFloat("u_Material.shininess", mat.GetShininess());

		mat.GetShader()->UploadFloat3("u_DirectionalLight.ambient", { 0.2f, 0.2f, 0.2f });
		mat.GetShader()->UploadFloat3("u_DirectionalLight.diffuse", { 0.5f, 0.5f, 0.5f });
		mat.GetShader()->UploadFloat3("u_DirectionalLight.specular", { 1.f, 1.f, 1.f });
		mat.GetShader()->UploadFloat3("u_DirectionalLight.direction", { 1.f, -1.f, 0.5f });

		mat.GetShader()->UploadFloat("u_PointLight.constant", 1.f);
		mat.GetShader()->UploadFloat("u_PointLight.linear", 0.09f);
		mat.GetShader()->UploadFloat("u_PointLight.quadratic", 0.032f);
		mat.GetShader()->UploadFloat3("u_PointLight.position", { 5, 3, 2 });
		mat.GetShader()->UploadFloat3("u_PointLight.ambient", { 0.5f, 0.2f, 0.2f });
		mat.GetShader()->UploadFloat3("u_PointLight.diffuse", { 0.7f, 0.3f, 0.5f });
		mat.GetShader()->UploadFloat3("u_PointLight.specular", { 1.f, 1.f, 1.f });
		/////////////////

		mesh.GetVertexArray()->Bind();
		Renderer::DrawIndexed(mesh.GetVertexArray());
	}

	void Renderer3D::DrawSphere()
	{
		s_pData->pSphereArray->Bind();
		Renderer::DrawIndexed(s_pData->pSphereArray);
	}
	void Renderer3D::DrawLine(const glm::vec3& posA, const glm::vec3& posB, float width)
	{
		glLineWidth(width);

		if (s_pData->LineIndexCount >= Renderer3DStorage::MaxLineIndices)
		{
			StartNewBatch();
		}

		s_pData->pLineVertexBufferPtr->Position = posA;
		s_pData->pLineVertexBufferPtr->Color = glm::vec4(1.f, 1.f, 1.f, 1.f);
		s_pData->pLineVertexBufferPtr++;

		s_pData->pLineVertexBufferPtr->Position = posB;
		s_pData->pLineVertexBufferPtr->Color = glm::vec4(1.f, 1.f, 1.f, 1.f);
		s_pData->pLineVertexBufferPtr++;

		s_pData->LineIndexCount += 2;
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

		Ref<VertexBuffer> pBuffer = VertexBuffer::Create(vertices, sizeof(Vertex) * vertices.size());
		pBuffer->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal" },
			{ ElementType::Float2, "a_TexCoords" }
			});

		s_pData->pSphereArray->AddVertexBuffer(pBuffer);

		Ref<IndexBuffer> pIndexBuffer = IndexBuffer::Create(indices, indices.size());
		s_pData->pSphereArray->SetIndexBuffer(pIndexBuffer);

		s_pData->pSphereArray->Unbind();

	}
	void Renderer3D::StartNewBatch()
	{
		End();
		ResetBatchData();
	}
	void Renderer3D::ResetBatchData()
	{
		s_pData->LineIndexCount = 0;
		s_pData->pLineVertexBufferPtr = s_pData->pLineVertexBufferBase;
	}
}