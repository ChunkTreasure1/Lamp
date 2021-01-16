#include "lppch.h"
#include "Renderer3D.h"

#include <glad/glad.h>
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Lamp/Core/Application.h"
#include "Lamp/Rendering/Texture2D/Texture2D.h"
#include "Lamp/Rendering/TextureCube/TextureCube.h"

#include "RenderCommand.h"
#include "Lamp/Rendering/Shader/ShaderLibrary.h"
#include "Lamp/Rendering/Texture2D/SkyboxDraw.h"

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

		Ref<SkyboxDraw> TestSky;

		//////Lines//////
		Ref<VertexArray> LineVertexArray;
		Ref<VertexBuffer> LineVertexBuffer;
		uint32_t LineIndexCount = 1;

		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;
		/////////////////

		/////Grid/////
		Ref<VertexArray> GridVertexArray;
		Ref<Shader> GridShader;
		//////////////

		Renderer3DStorage()
			: LineMaterial(Lamp::ShaderLibrary::GetShader("Line"), 0)
		{}

		~Renderer3DStorage()
		{
			delete[] LineVertexBufferBase;
		}

		RenderPassInfo CurrentRenderPass;
		Ref<VertexArray> SphereArray;

		Material LineMaterial;
	};

	Ref<FrameBuffer> Renderer3D::m_pFrameBuffer = nullptr;
	Ref<FrameBuffer> Renderer3D::m_pShadowBuffer = nullptr;
	static Renderer3DStorage* s_pData;

	void Renderer3D::Initialize()
	{
		s_pData = new Renderer3DStorage();

		s_pData->TestSky = CreateRef<SkyboxDraw>("assets/textures/newport_loft.hdr");

		///////Line///////
		s_pData->LineVertexArray = VertexArray::Create();
		s_pData->LineVertexBuffer = VertexBuffer::Create(s_pData->MaxLineVerts * sizeof(LineVertex));
		s_pData->LineVertexBuffer->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float4, "a_Color" }
		});
		s_pData->LineVertexArray->AddVertexBuffer(s_pData->LineVertexBuffer);
		s_pData->LineVertexBufferBase = new LineVertex[s_pData->MaxLineVerts];

		uint32_t* pLineIndices = new uint32_t[s_pData->MaxLineIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_pData->MaxLineIndices; i += 2)
		{
			pLineIndices[i + 0] = offset + 0;
			pLineIndices[i + 1] = offset + 1;

			offset += 2;
		}

		Ref<IndexBuffer> pLineIB = IndexBuffer::Create(pLineIndices, s_pData->MaxLineIndices);
		s_pData->LineVertexArray->SetIndexBuffer(pLineIB);

		delete[] pLineIndices;
		//////////////////

		/////Grid/////
		std::vector<float> gridPos =
		{
			-1, -1, 0,
			 1, -1, 0,
			 1,  1, 0,
			-1, 1, 0,
		};

		std::vector<uint32_t> gridIndices =
		{
			0, 1, 3,
			1, 2, 3
		};

		s_pData->GridVertexArray = VertexArray::Create();
		Ref<VertexBuffer> buffer = VertexBuffer::Create(gridPos, (uint32_t)(sizeof(float) * gridPos.size()));
		buffer->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" }
		});
		s_pData->GridVertexArray->AddVertexBuffer(buffer);

		Ref<IndexBuffer> gridIndexBuffer = IndexBuffer::Create(gridIndices, (uint32_t)(gridIndices.size()));
		s_pData->GridVertexArray->SetIndexBuffer(gridIndexBuffer);

		s_pData->GridVertexArray->Unbind();
		//////////////

		m_pFrameBuffer = Lamp::FrameBuffer::Create(1280, 720);
		m_pShadowBuffer = Lamp::FrameBuffer::Create(1024, 1024);
	}

	void Renderer3D::Shutdown()
	{
		delete s_pData;
	}

	void Renderer3D::Begin(const RenderPassInfo& passInfo)
	{
		s_pData->CurrentRenderPass = passInfo;

		ResetBatchData();
	}

	void Renderer3D::End()
	{
		uint32_t dataSize = (uint8_t*)s_pData->LineVertexBufferPtr - (uint8_t*)s_pData->LineVertexBufferBase;
		s_pData->LineVertexBuffer->SetData(s_pData->LineVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer3D::Flush()
	{
		s_pData->LineMaterial.GetShader()->Bind();
		s_pData->LineMaterial.GetShader()->UploadMat4("u_ViewProjection", s_pData->CurrentRenderPass.ViewProjection);

		RenderCommand::DrawIndexedLines(s_pData->LineVertexArray, s_pData->LineIndexCount);
	}

	void Renderer3D::DrawMesh(const glm::mat4& modelMatrix, Ref<Mesh>& mesh, Material& mat)
	{
		int i = 0;
		for (auto& name : mat.GetShader()->GetSpecifications().TextureNames)
		{
			if (mat.GetTextures()[name].get() != nullptr)
			{
				mat.GetTextures()[name]->Bind(i);
				i++;
			}
		}

		mat.GetShader()->Bind();
		mat.GetShader()->UploadFloat3("u_CameraPosition", s_pData->CurrentRenderPass.Camera->GetPosition());
		mat.GetShader()->UploadMat4("u_Model", modelMatrix);	
		mat.GetShader()->UploadMat4("u_ViewProjection", s_pData->CurrentRenderPass.ViewProjection);

		mesh->GetVertexArray()->Bind();
		RenderCommand::DrawIndexed(mesh->GetVertexArray(), mesh->GetVertexArray()->GetIndexBuffer()->GetCount());
	}

	void Renderer3D::DrawSkybox()
	{
		//s_pData->TestSky->Draw(s_pData->CurrentRenderPass.Camera->GetViewMatrix());
	}

	void Renderer3D::DrawGrid()
	{
		s_pData->GridShader->Bind();
		s_pData->GridVertexArray->Bind();

		glm::mat4 viewMat = glm::mat4(glm::mat3(s_pData->CurrentRenderPass.Camera->GetViewMatrix()));
		s_pData->GridShader->UploadMat4("u_View", viewMat);

		s_pData->GridShader->UploadMat4("u_Projection", s_pData->CurrentRenderPass.ViewProjection);
		RenderCommand::DrawIndexed(s_pData->GridVertexArray, 0);
	}

	void Renderer3D::DrawLine(const glm::vec3& posA, const glm::vec3& posB, float width)
	{
		glLineWidth(width);

		if (s_pData->LineIndexCount >= Renderer3DStorage::MaxLineIndices)
		{
			StartNewBatch();
		}

		s_pData->LineVertexBufferPtr->Position = posA;
		s_pData->LineVertexBufferPtr->Color = glm::vec4(1.f, 1.f, 1.f, 1.f);
		s_pData->LineVertexBufferPtr++;

		s_pData->LineVertexBufferPtr->Position = posB;
		s_pData->LineVertexBufferPtr->Color = glm::vec4(1.f, 1.f, 1.f, 1.f);
		s_pData->LineVertexBufferPtr++;

		s_pData->LineIndexCount += 2;
	}

	void Renderer3D::StartNewBatch()
	{
		End();
		ResetBatchData();
	}

	void Renderer3D::ResetBatchData()
	{
		s_pData->LineIndexCount = 0;
		s_pData->LineVertexBufferPtr = s_pData->LineVertexBufferBase;
	}
}