#include "lppch.h"
#include "Renderer.h"

#include "Vertices/VertexArray.h"
#include "Renderer2D.h"

#include "RenderCommand.h"

namespace Lamp
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		//TODO: maskid, tiling factor
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
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

		///////Lines///////
		Ref<VertexArray> pLineVertexArray;
		Ref<VertexBuffer> pLineVertexBuffer;
		uint32_t LineIndexCount = 0;

		LineVertex* pLineVertexBufferBase = nullptr;
		LineVertex* pLineVertexBufferPtr = nullptr;
		///////////////////

		Ref<Shader> pTextureShader;
		Ref<Texture2D> pWhiteTexture;
		Ref<Texture2D>* TextureSlots{ new Ref<Texture2D>[RenderCommand::GetCapabilities().MaxTextureSlots] };

		uint32_t TextureSlotIndex = 1; // 0 = white texture
		Renderer2D::Statistics Stats;
	};

	Ref<FrameBuffer> Renderer2D::m_pFrameBuffer = nullptr;
	static Renderer2DStorage* s_pData;

	void Renderer2D::Initialize()
	{
		s_pData = new Renderer2DStorage();

		////////Quad////////
		s_pData->pQuadVertexArray = VertexArray::Create();

		s_pData->pQuadVertexBuffer = VertexBuffer::Create(s_pData->MaxVertices * sizeof(QuadVertex));
		s_pData->pQuadVertexBuffer->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float4, "a_Color" },
			{ ElementType::Float2, "a_TexCoord" },
			{ ElementType::Float, "a_TexIndex" }
		});
		s_pData->pQuadVertexArray->AddVertexBuffer(s_pData->pQuadVertexBuffer);

		s_pData->QuadVertexBufferBase = new QuadVertex[s_pData->MaxVertices];

		uint32_t* pQuadIndices = new uint32_t[s_pData->MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_pData->MaxIndices; i += 6)
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
		pQuadIB = IndexBuffer::Create(pQuadIndices, s_pData->MaxIndices);
		s_pData->pQuadVertexArray->SetIndexBuffer(pQuadIB);

		s_pData->QuadVertexPositions[0] = { -0.5f, -0.5, 0.f, 1.f };
		s_pData->QuadVertexPositions[1] = { 0.5f, -0.5, 0.f, 1.f };
		s_pData->QuadVertexPositions[2] = { 0.5f, 0.5, 0.f, 1.f };
		s_pData->QuadVertexPositions[3] = { -0.5f, 0.5, 0.f, 1.f };

		delete[] pQuadIndices;
		////////////////////

		////////Line////////
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
		offset = 0;
		for (uint32_t i = 0; i < s_pData->MaxLineIndices; i += 2)
		{
			pLineIndices[i + 0] = offset + 0;
			pLineIndices[i + 1] = offset + 1;

			offset += 2;
		}

		Ref<IndexBuffer> pLineIB;
		pLineIB = IndexBuffer::Create(pLineIndices, s_pData->MaxLineIndices);
		s_pData->pLineVertexArray->SetIndexBuffer(pLineIB);

		delete[] pLineIndices;
		////////////////////

		////////Textures////////
		s_pData->pWhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_pData->pWhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int* samplers{ new int[RenderCommand::GetCapabilities().MaxTextureSlots] };
		for (int i = 0; i < RenderCommand::GetCapabilities().MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}

		s_pData->pTextureShader = Shader::Create({ {"engine/shaders/2d/Texture.vert"}, {"engine/shaders/2d/Texture.frag"} });
		s_pData->pTextureShader->Bind();
		s_pData->pTextureShader->UploadIntArray("u_Textures", samplers, RenderCommand::GetCapabilities().MaxTextureSlots);

		s_pData->TextureSlots[0] = s_pData->pWhiteTexture;

		delete[] samplers;
		////////////////////////

		m_pFrameBuffer = Lamp::FrameBuffer::Create(1280, 720);
	}

	void Renderer2D::Shutdown()
	{

		delete[] s_pData->TextureSlots;
		delete s_pData;
	}

	void Renderer2D::Begin(const Ref<CameraBase>& camera)
	{
		m_pFrameBuffer->Bind();
		RenderCommand::Clear();

		s_pData->pTextureShader->Bind();
		s_pData->pTextureShader->UploadMat4("u_ViewProjection", camera->GetViewProjectionMatrix());

		ResetBatchData();
	}

	void Renderer2D::End()
	{
		uint32_t dataSize = (uint8_t*)s_pData->QuadVertexBufferPtr - (uint8_t*)s_pData->QuadVertexBufferBase;
		s_pData->pQuadVertexBuffer->SetData(s_pData->QuadVertexBufferBase, dataSize);

		dataSize = (uint8_t*)s_pData->pLineVertexBufferPtr - (uint8_t*)s_pData->pLineVertexBufferBase;
		s_pData->pLineVertexBuffer->SetData(s_pData->pLineVertexBufferBase, dataSize);

		Flush();
		m_pFrameBuffer->Unbind();
	}

	void Renderer2D::Flush()
	{
		//Bind textures
		for (uint32_t i = 0; i < s_pData->TextureSlotIndex; i++)
		{
			s_pData->TextureSlots[i]->Bind(i);
		}

		//Draw
		RenderCommand::DrawIndexed(s_pData->pQuadVertexArray, s_pData->QuadIndexCount);
		s_pData->Stats.DrawCalls++;

		RenderCommand::DrawIndexedLines(s_pData->pLineVertexArray, s_pData->LineIndexCount);
		s_pData->Stats.DrawCalls++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& tm, const glm::vec4& color)
	{

		if (s_pData->QuadIndexCount >= Renderer2DStorage::MaxIndices)
		{
			StartNewBatch();
		}

		const float texIndex = 0.f; // white texture

		s_pData->QuadVertexBufferPtr->Position = tm * s_pData->QuadVertexPositions[0];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 0.f, 0.f };
		s_pData->QuadVertexBufferPtr->TexIndex = texIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadVertexBufferPtr->Position = tm * s_pData->QuadVertexPositions[1];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 1.f, 0.f };
		s_pData->QuadVertexBufferPtr->TexIndex = texIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadVertexBufferPtr->Position = tm * s_pData->QuadVertexPositions[2];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 1.f, 1.f };
		s_pData->QuadVertexBufferPtr->TexIndex = texIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadVertexBufferPtr->Position = tm * s_pData->QuadVertexPositions[3];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 0.f, 1.f };
		s_pData->QuadVertexBufferPtr->TexIndex = texIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadIndexCount += 6;
		s_pData->Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& tm, const Ref<Texture2D>& texture, const glm::vec4& color)
	{

		if (s_pData->QuadIndexCount >= Renderer2DStorage::MaxIndices)
		{
			StartNewBatch();
		}

		float textureIndex = 0.f;

		for (uint32_t i = 1; i < s_pData->TextureSlotIndex; i++)
		{
			//TODO: change
			if (s_pData->TextureSlots[i].get() == texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.f)
		{
			textureIndex = (float)s_pData->TextureSlotIndex;
			s_pData->TextureSlots[s_pData->TextureSlotIndex] = texture;
			s_pData->TextureSlotIndex++;
		}

		s_pData->QuadVertexBufferPtr->Position = tm * s_pData->QuadVertexPositions[0];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 0.f, 0.f };
		s_pData->QuadVertexBufferPtr->TexIndex = textureIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadVertexBufferPtr->Position = tm * s_pData->QuadVertexPositions[1];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 1.f, 0.f };
		s_pData->QuadVertexBufferPtr->TexIndex = textureIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadVertexBufferPtr->Position = tm * s_pData->QuadVertexPositions[2];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 1.f, 1.f };
		s_pData->QuadVertexBufferPtr->TexIndex = textureIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadVertexBufferPtr->Position = tm * s_pData->QuadVertexPositions[3];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 0.f, 1.f };
		s_pData->QuadVertexBufferPtr->TexIndex = textureIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadIndexCount += 6;

		s_pData->Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec2& scale, const glm::vec4& color)
	{

		if (s_pData->QuadIndexCount >= Renderer2DStorage::MaxIndices)
		{
			StartNewBatch();
		}

		const float texIndex = 0.f; // white texture

		glm::mat4 transform = glm::translate(glm::mat4(1.f), pos)
			* glm::scale(glm::mat4(1.f), { scale.x, scale.y, 1.f });

		s_pData->QuadVertexBufferPtr->Position = transform * s_pData->QuadVertexPositions[0];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 0.f, 0.f };
		s_pData->QuadVertexBufferPtr->TexIndex = texIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadVertexBufferPtr->Position = transform * s_pData->QuadVertexPositions[1];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 1.f, 0.f };
		s_pData->QuadVertexBufferPtr->TexIndex = texIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadVertexBufferPtr->Position = transform * s_pData->QuadVertexPositions[2];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 1.f, 1.f };
		s_pData->QuadVertexBufferPtr->TexIndex = texIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadVertexBufferPtr->Position = transform * s_pData->QuadVertexPositions[3];
		s_pData->QuadVertexBufferPtr->Color = color;
		s_pData->QuadVertexBufferPtr->TexCoord = { 0.f, 1.f };
		s_pData->QuadVertexBufferPtr->TexIndex = texIndex;
		s_pData->QuadVertexBufferPtr++;

		s_pData->QuadIndexCount += 6;

		s_pData->Stats.QuadCount++;
	}

	void Renderer2D::DrawLine(const glm::vec3& posA, const glm::vec3& posB)
	{
		if (s_pData->LineIndexCount >= Renderer2DStorage::MaxLineIndices)
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
		s_pData->Stats.LineCount++;
	}

	void Renderer2D::DrawLine(const glm::vec2& posA, const glm::vec2& posB)
	{
		glLineWidth(2.f);
		DrawLine({ posA.x, posA.y, 0.f }, { posB.x, posB.y, 0.f });
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{

		return s_pData->Stats;
	}

	void Renderer2D::ResetStats()
	{

		memset(&s_pData->Stats, 0, sizeof(Renderer2D::Statistics));
	}

	void Renderer2D::StartNewBatch()
	{
		End();
		ResetBatchData();
	}

	void Renderer2D::ResetBatchData()
	{
		s_pData->QuadIndexCount = 0;
		s_pData->QuadVertexBufferPtr = s_pData->QuadVertexBufferBase;
		s_pData->TextureSlotIndex = 1;

		s_pData->LineIndexCount = 0;
		s_pData->pLineVertexBufferPtr = s_pData->pLineVertexBufferBase;
	}
}