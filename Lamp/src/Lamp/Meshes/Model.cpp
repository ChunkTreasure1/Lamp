#include "lppch.h"
#include "Model.h"

#include "Lamp/Rendering/Renderer3D.h"

namespace Lamp
{
	void Model::Draw()
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
		{
			m_Material.GetShader()->Bind();

			m_Material.GetShader()->UploadFloat3("u_DirLight.direction", { -0.2f, -1.f, 0.3f });
			m_Material.GetShader()->UploadFloat3("u_DirLight.ambient", { 0.05f, 0.05f, 0.05f });
			m_Material.GetShader()->UploadFloat3("u_DirLight.diffuse", { 0.4f, 0.4f, 0.4f });
			m_Material.GetShader()->UploadFloat3("u_DirLight.specular", { 1.f, 1.f, 1.f });

			m_Material.GetShader()->UploadFloat3("u_PointLight.position", { -1.f, 0.4f, 2.f });
			m_Material.GetShader()->UploadFloat3("u_PointLight.ambient", { 0.05f, 0.05f, 0.05f });
			m_Material.GetShader()->UploadFloat3("u_PointLight.diffuse", { 0.8f, 0.8f, 0.8f });
			m_Material.GetShader()->UploadFloat3("u_PointLight.specular", { 1.f, 1.f, 1.f });
			m_Material.GetShader()->UploadFloat("u_PointLight.constant", 1.f);
			m_Material.GetShader()->UploadFloat("u_PointLight.linear", 0.09f);
			m_Material.GetShader()->UploadFloat("u_PointLight.quadric", 0.032f);

			m_Material.GetShader()->UploadFloat3("u_SpotLight.ambient", { 0.f, 0.f, 0.f });
			m_Material.GetShader()->UploadFloat3("u_SpotLight.diffuse", { 1.f, 1.f, 1.f });
			m_Material.GetShader()->UploadFloat3("u_SpotLight.specular", { 1.f, 1.f, 1.f });

			m_Material.GetShader()->UploadFloat("u_SpotLight.constant", 1.f);
			m_Material.GetShader()->UploadFloat("u_SpotLight.linear", 0.09f);
			m_Material.GetShader()->UploadFloat("u_SpotLight.quadratic", 0.032f);
			m_Material.GetShader()->UploadFloat("u_SpotLight.cutOff", glm::cos(glm::radians(12.5f)));
			m_Material.GetShader()->UploadFloat("u_SpotLight.outerCutOff", glm::cos(glm::radians(15.f)));
			
			m_Material.GetShader()->UploadInt("u_Material.diffuse", 0);
			m_Material.GetShader()->UploadInt("u_Material.specular", 1);
			m_Material.GetShader()->UploadFloat("u_Material.shininess", 32.0f);

			Renderer3D::DrawMesh(m_ModelMatrix, m_Meshes[i], m_Material);
		}
	}
}