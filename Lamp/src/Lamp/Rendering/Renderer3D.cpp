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
	};

	static Renderer3DStorage* s_pData;

	void Renderer3D::Initialize()
	{
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);

		s_pData = new Renderer3DStorage();
	}
	void Renderer3D::Shutdown()
	{
	}
	void Renderer3D::Begin(PerspectiveCamera& camera)
	{
		s_pData->pCamera = &camera;
	}
	void Renderer3D::End()
	{
	}
	void Renderer3D::TestDraw()
	{
		//glActiveTexture(GL_TEXTURE0);
		//s_pData->pTexture->Bind();
		//glActiveTexture(GL_TEXTURE1);
		//s_pData->pSpecTexture->Bind();

		//s_pData->pShader->Bind();

		//// create transformations
		//glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first

		//s_pData->pShader->UploadMat4("u_Model", model);

		//// render container
		//s_pData->pVertexArray->Bind();
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		//s_pData->pLightShader->Bind();
		//glm::mat4 lightModel = glm::mat4(1.f);
		//lightModel = glm::translate(lightModel, { 1.2f, 1.f, -2.f });
		//lightModel = glm::scale(lightModel, glm::vec3(0.2f));

		//s_pData->pLightShader->UploadMat4("u_Model", lightModel);

		//s_pData->pLightSource->Bind();
		//glDrawArrays(GL_TRIANGLES, 0, 36);
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
}