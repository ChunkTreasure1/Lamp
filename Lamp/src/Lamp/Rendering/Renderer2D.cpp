#include "lppch.h"
#include "Renderer.h"

#include "Vertecies/VertexArray.h"
#include "Renderer2D.h"

namespace Lamp
{
	struct Renderer2DStorage
	{
		Ref<VertexArray> pQuadVertexArray;
		Ref<Shader> pTextureShader;
	};

	static Renderer2DStorage* s_pData;
	static uint32_t VBO, VAO, EBO;

	void Renderer2D::Initialize()
	{
		s_pData = new Renderer2DStorage();
		s_pData->pQuadVertexArray = VertexArray::Create();

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.f, 0.f,
			 0.5f, -0.5f, 0.0f, 1.f, 0.f,
			 0.5f,  0.5f, 0.0f, 1.f, 1.f,
			-0.5f,  0.5f, 0.0f, 0.f, 1.f
		};

		Ref<VertexBuffer> pSquareVB;
		pSquareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		s_pData->pQuadVertexArray->AddVertexBuffer(pSquareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Ref<IndexBuffer> pSquareIB;
		pSquareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		s_pData->pQuadVertexArray->SetIndexBuffer(pSquareIB);

		s_pData->pTextureShader = Shader::Create("Assets/Shaders/colorShading.vert", "Assets/Shaders/colorShading.frag");
		s_pData->pTextureShader->UploadInt("u_Texture", 0);
	}

	void Renderer2D::Shutdown()
	{
		delete s_pData;
	}

	void Renderer2D::Begin(const OrthographicCamera& camera)
	{
		s_pData->pTextureShader->Bind();
		s_pData->pTextureShader->UploadMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		s_pData->pTextureShader->UploadMat4("u_Transform", glm::mat4(1.f));
	}

	void Renderer2D::End()
	{}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec2& scale, const glm::vec4& color)
	{
		s_pData->pTextureShader->Bind();
		s_pData->pTextureShader->UploadFloat4("u_Color", color);

		glm::mat4 transform = glm::translate(glm::mat4(1.f), pos) * glm::scale(glm::mat4(1.f), { scale.x, scale.y, 1.f });
		s_pData->pTextureShader->UploadMat4("u_Transform", transform);

		s_pData->pQuadVertexArray->Bind();
		Renderer::DrawIndexed(s_pData->pQuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& pos, const glm::vec2& scale, const glm::vec4& color)
	{
		DrawQuad({ pos.x, pos.y, 0.f }, scale, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& pos, const glm::vec2& scale, const Ref<Texture2D>& texture)
	{
		s_pData->pTextureShader->Bind();
		s_pData->pTextureShader->UploadInt("u_Texture", 0);

		glm::mat4 transform = glm::translate(glm::mat4(1.f), pos) * glm::scale(glm::mat4(1.f), { scale.x, scale.y, 1.f });
		s_pData->pTextureShader->UploadMat4("u_Transform", transform);

		texture->Bind();

		s_pData->pQuadVertexArray->Bind();
		Renderer::DrawIndexed(s_pData->pQuadVertexArray);
	}

	void Renderer2D::DrawQuad(const glm::vec2& pos, const glm::vec2& scale, const Ref<Texture2D>& texture)
	{
		DrawQuad({ pos.x, pos.y, 0.f }, scale, texture);
	}
}