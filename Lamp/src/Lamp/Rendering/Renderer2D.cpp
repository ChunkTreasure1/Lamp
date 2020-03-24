#include "lppch.h"
#include "Renderer.h"

#include "Vertices/VertexArray.h"
#include "Renderer2D.h"

namespace Lamp
{
	struct Renderer2DStorage
	{
		Ref<VertexArray> pQuadVertexArray;
		Ref<VertexArray> pLineVertexArray;
		Ref<Shader> pTextureShader;
		Ref<Texture2D> pWhiteTexture;
	};

	static Renderer2DStorage* s_pData;
	static uint32_t VBO, VAO, EBO;

	void Renderer2D::Initialize()
	{
		//s_pData = new Renderer2DStorage();

		//////////Square////////
		//s_pData->pQuadVertexArray = VertexArray::Create();

		//float squareVertices[5 * 4] = {
		//	-0.5f, -0.5f, 0.0f, 0.f, 0.f,
		//	 0.5f, -0.5f, 0.0f, 1.f, 0.f,
		//	 0.5f,  0.5f, 0.0f, 1.f, 1.f,
		//	-0.5f,  0.5f, 0.0f, 0.f, 1.f
		//};

		//Ref<VertexBuffer> pSquareVB;
		//pSquareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		//pSquareVB->SetBufferLayout
		//({
		//	{ ElementType::Float3, "a_Position" },
		//	{ ElementType::Float2, "a_TexCoord" }
		//	});
		//s_pData->pQuadVertexArray->AddVertexBuffer(pSquareVB);

		//uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		//Ref<IndexBuffer> pSquareIB;
		//pSquareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		//s_pData->pQuadVertexArray->SetIndexBuffer(pSquareIB);

		//////////Lines////////
		//s_pData->pLineVertexArray = VertexArray::Create();
		//Ref<VertexBuffer> pLineVB;
		//pLineVB.reset(VertexBuffer::Create(nullptr, 0.f));
		//pLineVB->SetBufferLayout
		//({
		//	{ ElementType::Float3, "a_Position" }
		//});
		//s_pData->pLineVertexArray->AddVertexBuffer(pLineVB);

		//uint32_t lineIndices[2] = { 0, 1 };
		//Ref<IndexBuffer> pLineIB;
		//pLineIB.reset(IndexBuffer::Create(lineIndices, sizeof(lineIndices) / sizeof(uint32_t)));
		//s_pData->pLineVertexArray->SetIndexBuffer(pSquareIB);

		//s_pData->pWhiteTexture = Texture2D::Create(1, 1);
		//uint32_t whiteTextureData = 0xffffffff;
		//s_pData->pWhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		//s_pData->pTextureShader = Shader::Create("engine/shaders/Texture.vert", "engine/shaders/Texture.frag");
		//s_pData->pTextureShader->UploadInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		delete s_pData;
	}

	void Renderer2D::Begin(const OrthographicCamera camera)
	{
		s_pData->pTextureShader->Bind();
		s_pData->pTextureShader->UploadMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer2D::End() {}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec2& scale, const glm::vec4& color)
	{
		s_pData->pTextureShader->UploadFloat4("u_Color", color);
		s_pData->pWhiteTexture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.f), pos) * glm::scale(glm::mat4(1.f), { scale.x, scale.y, 1.f });
		s_pData->pTextureShader->UploadMat4("u_Transform", transform);

		s_pData->pQuadVertexArray->Bind();
		Renderer::DrawIndexed(s_pData->pQuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& pos, const glm::vec2& scale, const glm::vec4& color)
	{
		DrawQuad({ pos.x, pos.y, 0.f }, scale, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec2& scale, float rotation, const glm::vec4& color)
	{
		s_pData->pTextureShader->UploadFloat4("u_Color", color);
		s_pData->pWhiteTexture->Bind();

		glm::mat4 transform = glm::translate(glm::mat4(1.f), pos)
			* glm::rotate(glm::mat4(1.f), rotation, { 0.f, 0.f, 1.f })
			* glm::scale(glm::mat4(1.f), { scale.x, scale.y, 1.f });
		s_pData->pTextureShader->UploadMat4("u_Transform", transform);

		s_pData->pQuadVertexArray->Bind();
		Renderer::DrawIndexed(s_pData->pQuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& pos, const glm::vec2& scale, float rotation, const glm::vec4& color)
	{
		DrawQuad({ pos.x, pos.y, 0 }, scale, rotation, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec2& scale, const Ref<Texture2D>& texture, const glm::vec4& color)
	{

		texture->Bind();

		s_pData->pTextureShader->UploadInt("u_Texture", 0);
		s_pData->pTextureShader->UploadFloat4("u_Color", color);

		glm::mat4 transform = glm::translate(glm::mat4(1.f), pos) * glm::scale(glm::mat4(1.f), { scale.x, scale.y, 1.f });
		s_pData->pTextureShader->UploadMat4("u_Transform", transform);

		s_pData->pQuadVertexArray->Bind();
		Renderer::DrawIndexed(s_pData->pQuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& pos, const glm::vec2& scale, const Ref<Texture2D>& texture, const glm::vec4& color)
	{
		DrawQuad({ pos.x, pos.y, 0.f }, scale, texture, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec2& scale, float rotation, const Ref<Texture2D>& texture, const glm::vec4& color)
	{
		texture->Bind();

		s_pData->pTextureShader->UploadInt("u_Texture", 0);
		s_pData->pTextureShader->UploadFloat4("u_Color", color);

		glm::mat4 transform = glm::translate(glm::mat4(1.f), pos)
			* glm::rotate(glm::mat4(1.f), rotation, { 0.f, 0.f, 1.f })
			* glm::scale(glm::mat4(1.f), { scale.x, scale.y, 1.f });
		s_pData->pTextureShader->UploadMat4("u_Transform", transform);

		s_pData->pQuadVertexArray->Bind();
		Renderer::DrawIndexed(s_pData->pQuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& pos, const glm::vec2& scale, float rotation, const Ref<Texture2D>& texture, const glm::vec4& color)
	{
		DrawQuad({ pos.x, pos.y, 0.f }, scale, rotation, texture, color);
	}
	
	void Renderer2D::DrawLine(const glm::vec3& posA, const glm::vec3& posB)
	{
		float verts[3 * 2] = {
			posA.x, posA.y, posA.z,
			posB.x, posB.y, posB.z
		};

		s_pData->pLineVertexArray->GetVertexBuffer()[0]->SetVertices(verts, sizeof(verts));

		s_pData->pTextureShader->UploadFloat4("u_Color", { 1.f, 1.f, 1.f, 1.f });
		s_pData->pWhiteTexture->Bind();

		glm::mat4 transform = glm::mat4(1.f);
		s_pData->pTextureShader->UploadMat4("u_Transform", transform);

		s_pData->pLineVertexArray->Bind();
		Renderer::DrawIndexedLines(s_pData->pLineVertexArray);
	}

	void Renderer2D::DrawLine(const glm::vec2& posA, const glm::vec2& posB)
	{
		glLineWidth(3.f);
		DrawLine({ posA.x, posA.y, 0.f }, { posB.x, posB.y, 0.f });
	}
}