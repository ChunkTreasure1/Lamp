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
		Ref<VertexArray> pVertexArray;
		Ref<VertexArray> pLightSource;

		Ref<Shader> pLightShader;
		Ref<Shader> pShader;
		Ref<Texture2D> pTexture;
		Ref<Texture2D> pSpecTexture;
	};

	static Renderer3DStorage* s_pData;

	void Renderer3D::Initialize()
	{
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);

		s_pData = new Renderer3DStorage();
		s_pData->pShader = std::make_shared<Shader>("engine/shaders/shader_vs.glsl", "engine/shaders/shader_fs.glsl");
		s_pData->pLightShader = std::make_shared<Shader>("engine/shaders/lightShader.vs", "engine/shaders/lightShader.fs");

		float vertices[] = {
			// positions          // normals           // texture coords
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
		};

		float lightVerts[] = {
			// positions        coords
			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			-0.5f,  0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,

			-0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f, -0.5f,  0.5f,

			-0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,

			 0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,

			-0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f,  0.5f,
			-0.5f, -0.5f,  0.5f,
			-0.5f, -0.5f, -0.5f,

			-0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f, -0.5f,
			 0.5f,  0.5f,  0.5f,
			 0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f,  0.5f,
			-0.5f,  0.5f, -0.5f,
		};

		s_pData->pVertexArray = VertexArray::Create();
		s_pData->pLightSource = VertexArray::Create();

		Ref<VertexBuffer> pBuffer;
		pBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		pBuffer->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" },
			{ ElementType::Float3, "a_Normal"},
			{ ElementType::Float2, "a_TexCoords" }
		});

		s_pData->pVertexArray->AddVertexBuffer(pBuffer);

		Ref<VertexBuffer> pLightBuf;
		pLightBuf.reset(VertexBuffer::Create(lightVerts, sizeof(lightVerts)));
		pLightBuf->SetBufferLayout
		({
			{ ElementType::Float3, "a_Position" }
			});
		s_pData->pLightSource->AddVertexBuffer(pLightBuf);

		s_pData->pTexture = Texture2D::Create("engine/textures/container_diff.png");
		s_pData->pSpecTexture = Texture2D::Create("engine/textures/container_spec.png");

		s_pData->pShader->Bind();
		s_pData->pShader->UploadInt("u_Material.diffuse", 0);
		s_pData->pShader->UploadInt("u_Material.specular", 1);
	}
	void Renderer3D::Shutdown()
	{
	}
	void Renderer3D::Begin(PerspectiveCamera& camera)
	{
		s_pData->pShader->Bind();
		s_pData->pShader->UploadMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		s_pData->pShader->UploadFloat3("u_CameraPosition", camera.GetPosition());

		s_pData->pLightShader->Bind();
		s_pData->pLightShader->UploadMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

	}
	void Renderer3D::End()
	{
	}
	void Renderer3D::TestDraw()
	{
		glActiveTexture(GL_TEXTURE0);
		s_pData->pTexture->Bind();
		glActiveTexture(GL_TEXTURE1);
		s_pData->pSpecTexture->Bind();

		s_pData->pShader->Bind();

		// create transformations
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

		s_pData->pShader->UploadMat4("u_Model", model);

		//Light
		s_pData->pShader->UploadFloat3("u_Light.ambient", { 0.5f, 0.5f, 0.5f });
		s_pData->pShader->UploadFloat3("u_Light.specular", { 1.f, 1.f, 1.f });
		s_pData->pShader->UploadFloat3("u_Light.position", { 1.2f, 1.f, -2.f });

		//Material
		s_pData->pShader->UploadFloat3("u_Material.ambient", { 1.0f, 0.5f, 0.31f });
		s_pData->pShader->UploadFloat3("u_Material.diffuse", { 1.0f, 0.5f, 0.31f });
		s_pData->pShader->UploadFloat3("u_Material.specular", { 0.5f, 0.5f, 0.5f });
		s_pData->pShader->UploadFloat("u_Material.shininess", 32.0f);

		// render container
		s_pData->pVertexArray->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);

		s_pData->pLightShader->Bind();
		glm::mat4 lightModel = glm::mat4(1.f);
		lightModel = glm::translate(lightModel, { 1.2f, 1.f, -2.f });
		lightModel = glm::scale(lightModel, glm::vec3(0.2f));

		s_pData->pLightShader->UploadMat4("u_Model", lightModel);

		s_pData->pLightSource->Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}