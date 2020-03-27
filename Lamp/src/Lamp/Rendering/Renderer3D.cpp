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

	Ref<FrameBuffer> Renderer3D::m_pFrameBuffer = nullptr;
	static Renderer3DStorage* s_pData;

	void Renderer3D::Initialize()
	{
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_DEPTH_TEST);

		s_pData = new Renderer3DStorage();

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
}