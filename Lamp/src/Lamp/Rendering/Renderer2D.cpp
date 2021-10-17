#include "lppch.h"
#include "Renderer.h"

#include "Vertices/VertexArray.h"
#include "Renderer2D.h"
#include "Shader/ShaderLibrary.h"

#include "RenderCommand.h"

namespace Lamp
{
	struct QuadVertex
	{
		glm::vec3 position;
		glm::vec4 color;
		glm::vec2 texCoord;
		float texIndex;
		//TODO: maskid, tiling factor
	};

	struct Renderer2DStorage
	{
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;

		static const uint32_t MaxLines = 10000;
		static const uint32_t MaxLineVerts = MaxLines * 2;
		static const uint32_t MaxLineIndices = MaxLines * 2;

		///////Quads///////
		Ref<VertexArray> pQuadVertexArray;
		Ref<VertexBuffer> pQuadVertexBuffer;
		uint32_t QuadIndexCount = 0;

		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;
		glm::vec4 QuadVertexPositions[4];
		//////////////////

		Ref<Shader> pTextureShader;
		Ref<Texture2D> pWhiteTexture;
		Ref<Texture2D>* TextureSlots{ new Ref<Texture2D>[RenderCommand::GetCapabilities().MaxTextureSlots] };

		uint32_t TextureSlotIndex = 1; // 0 = white texture
		Renderer2D::Statistics Stats;
	};

	static Renderer2DStorage* s_pStorage;

	void Renderer2D::Initialize()
	{
		s_pStorage = new Renderer2DStorage();

		////////Quad////////
		s_pStorage->pQuadVertexArray = VertexArray::Create();

		s_pStorage->pQuadVertexBuffer = VertexBuffer::Create(s_pStorage->MaxVertices * sizeof(QuadVertex));
		s_pStorage->pQuadVertexBuffer->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float4, "a_Color" },
			{ ElementType::Float2, "a_TexCoord" },
			{ ElementType::Float, "a_TexIndex" }
		});
		s_pStorage->pQuadVertexArray->AddVertexBuffer(s_pStorage->pQuadVertexBuffer);

		s_pStorage->QuadVertexBufferBase = new QuadVertex[s_pStorage->MaxVertices];

		uint32_t* pQuadIndices = new uint32_t[s_pStorage->MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_pStorage->MaxIndices; i += 6)
		{
			pQuadIndices[i + 0] = offset + 0;
			pQuadIndices[i + 1] = offset + 1;
			pQuadIndices[i + 2] = offset + 2;

			pQuadIndices[i + 3] = offset + 2;
			pQuadIndices[i + 4] = offset + 3;
			pQuadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Ref<IndexBuffer> pQuadIB;
		pQuadIB = IndexBuffer::Create(pQuadIndices, s_pStorage->MaxIndices);
		s_pStorage->pQuadVertexArray->SetIndexBuffer(pQuadIB);

		s_pStorage->QuadVertexPositions[0] = { -0.5f, -0.5, 0.f, 1.f };
		s_pStorage->QuadVertexPositions[1] = { 0.5f, -0.5, 0.f, 1.f };
		s_pStorage->QuadVertexPositions[2] = { 0.5f, 0.5, 0.f, 1.f };
		s_pStorage->QuadVertexPositions[3] = { -0.5f, 0.5, 0.f, 1.f };

		delete[] pQuadIndices;
		////////////////////

		////////Textures////////
		s_pStorage->pWhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_pStorage->pWhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int* samplers{ new int[RenderCommand::GetCapabilities().MaxTextureSlots] };
		for (uint32_t i = 0; i < RenderCommand::GetCapabilities().MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}

		s_pStorage->pTextureShader = ShaderLibrary::GetShader("quad");
		s_pStorage->pTextureShader->Bind();
		s_pStorage->pTextureShader->UploadIntArray("u_Textures", samplers, RenderCommand::GetCapabilities().MaxTextureSlots);

		s_pStorage->TextureSlots[0] = s_pStorage->pWhiteTexture;

		delete[] samplers;
		////////////////////////
	}

	void Renderer2D::Shutdown()
	{
		delete[] s_pStorage->TextureSlots;
		delete s_pStorage;
	}

	void Renderer2D::Begin(const Ref<CameraBase>& camera)
	{
		RenderCommand::Clear();

		s_pStorage->pTextureShader->Bind();
		s_pStorage->pTextureShader->UploadMat4("u_ViewProjection", camera->GetViewProjectionMatrix());

		ResetBatchData();
	}

	void Renderer2D::End()
	{
		uint32_t dataSize = (uint8_t*)s_pStorage->QuadVertexBufferPtr - (uint8_t*)s_pStorage->QuadVertexBufferBase;
		s_pStorage->pQuadVertexBuffer->SetData(s_pStorage->QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::Flush()
	{
		//Bind textures
		for (uint32_t i = 0; i < s_pStorage->TextureSlotIndex; i++)
		{
			s_pStorage->TextureSlots[i]->Bind(i);
		}

		//Draw
		RenderCommand::DrawIndexed(s_pStorage->pQuadVertexArray, s_pStorage->QuadIndexCount);
		s_pStorage->Stats.drawCalls++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& tm, const glm::vec4& color)
	{
		if (s_pStorage->QuadIndexCount >= Renderer2DStorage::MaxIndices)
		{
			StartNewBatch();
		}

		const float texIndex = 0.f; // white texture

		s_pStorage->QuadVertexBufferPtr->position = (tm * s_pStorage->QuadVertexPositions[0]);
		s_pStorage->QuadVertexBufferPtr->color = color;
		s_pStorage->QuadVertexBufferPtr->texCoord = { 0.f, 0.f };
		s_pStorage->QuadVertexBufferPtr->texIndex = texIndex;
		s_pStorage->QuadVertexBufferPtr++;

		s_pStorage->QuadVertexBufferPtr->position = tm * s_pStorage->QuadVertexPositions[1];
		s_pStorage->QuadVertexBufferPtr->color = color;
		s_pStorage->QuadVertexBufferPtr->texCoord = { 1.f, 0.f };
		s_pStorage->QuadVertexBufferPtr->texIndex = texIndex;
		s_pStorage->QuadVertexBufferPtr++;

		s_pStorage->QuadVertexBufferPtr->position = tm * s_pStorage->QuadVertexPositions[2];
		s_pStorage->QuadVertexBufferPtr->color = color;
		s_pStorage->QuadVertexBufferPtr->texCoord = { 1.f, 1.f };
		s_pStorage->QuadVertexBufferPtr->texIndex = texIndex;
		s_pStorage->QuadVertexBufferPtr++;

		s_pStorage->QuadVertexBufferPtr->position = tm * s_pStorage->QuadVertexPositions[3];
		s_pStorage->QuadVertexBufferPtr->color = color;
		s_pStorage->QuadVertexBufferPtr->texCoord = { 0.f, 1.f };
		s_pStorage->QuadVertexBufferPtr->texIndex = texIndex;
		s_pStorage->QuadVertexBufferPtr++;

		s_pStorage->QuadIndexCount += 6;
		s_pStorage->Stats.quadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& tm, const Ref<Material> mat, uint32_t id, const glm::vec4& color)
	{
		if (s_pStorage->QuadIndexCount >= Renderer2DStorage::MaxIndices)
		{
			StartNewBatch();
		}

		float textureIndex = 0.f;

		auto& texture = mat->GetTextures().at("gizmo");

		for (uint32_t i = 1; i < s_pStorage->TextureSlotIndex; i++)
		{
			//TODO: change
			if (s_pStorage->TextureSlots[i].get() == texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.f)
		{
			textureIndex = (float)s_pStorage->TextureSlotIndex;
			s_pStorage->TextureSlots[s_pStorage->TextureSlotIndex] = texture;
			s_pStorage->TextureSlotIndex++;
		}

		s_pStorage->QuadVertexBufferPtr->position = tm * s_pStorage->QuadVertexPositions[0];
		s_pStorage->QuadVertexBufferPtr->color = color;
		s_pStorage->QuadVertexBufferPtr->texCoord = { 0.f, 0.f };
		s_pStorage->QuadVertexBufferPtr->texIndex = textureIndex;
		s_pStorage->QuadVertexBufferPtr++;

		s_pStorage->QuadVertexBufferPtr->position = tm * s_pStorage->QuadVertexPositions[1];
		s_pStorage->QuadVertexBufferPtr->color = color;
		s_pStorage->QuadVertexBufferPtr->texCoord = { 1.f, 0.f };
		s_pStorage->QuadVertexBufferPtr->texIndex = textureIndex;
		s_pStorage->QuadVertexBufferPtr++;

		s_pStorage->QuadVertexBufferPtr->position = tm * s_pStorage->QuadVertexPositions[2];
		s_pStorage->QuadVertexBufferPtr->color = color;
		s_pStorage->QuadVertexBufferPtr->texCoord = { 1.f, 1.f };
		s_pStorage->QuadVertexBufferPtr->texIndex = textureIndex;
		s_pStorage->QuadVertexBufferPtr++;

		s_pStorage->QuadVertexBufferPtr->position = tm * s_pStorage->QuadVertexPositions[3];
		s_pStorage->QuadVertexBufferPtr->color = color;
		s_pStorage->QuadVertexBufferPtr->texCoord = { 0.f, 1.f };
		s_pStorage->QuadVertexBufferPtr->texIndex = textureIndex;
		s_pStorage->QuadVertexBufferPtr++;

		s_pStorage->QuadIndexCount += 6;

		s_pStorage->Stats.quadCount++;
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_pStorage->Stats;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_pStorage->Stats, 0, sizeof(Renderer2D::Statistics));
	}

	void Renderer2D::StartNewBatch()
	{
		End();
		ResetBatchData();
	}

	void Renderer2D::ResetBatchData()
	{
		s_pStorage->QuadIndexCount = 0;
		s_pStorage->QuadVertexBufferPtr = s_pStorage->QuadVertexBufferBase;
		s_pStorage->TextureSlotIndex = 1;
	}
}