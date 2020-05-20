#include "lppch.h"
#include "Material.h"

namespace Lamp
{
	void Material::UploadData()
	{
		m_pShader->Bind();

		if (m_pShader->GetType() == ShaderType::Illum)
		{
			/////Lighting/////
			m_pShader->UploadFloat3("u_DirectionalLight.ambient", { 0.2f, 0.2f, 0.2f });
			m_pShader->UploadFloat3("u_DirectionalLight.diffuse", { 0.5f, 0.5f, 0.5f });
			m_pShader->UploadFloat3("u_DirectionalLight.specular", { 1.f, 1.f, 1.f });
			m_pShader->UploadFloat3("u_DirectionalLight.direction", { 1.f, -1.f, 0.5f });
			
			m_pShader->UploadFloat("u_PointLight.constant", 1.f);
			m_pShader->UploadFloat("u_PointLight.linear", 0.09f);
			m_pShader->UploadFloat("u_PointLight.quadratic", 0.032f);
			
			m_pShader->UploadFloat3("u_PointLight.position", { 0, 7, 0 });
			m_pShader->UploadFloat3("u_PointLight.ambient", { 0.2f, 0.2f, 0.2f });
			m_pShader->UploadFloat3("u_PointLight.diffuse", { 3.f, 3.f, 3.f });
			m_pShader->UploadFloat3("u_PointLight.specular", { 1.f, 1.f, 1.f });
			/////////////////

			m_pShader->UploadInt("u_Material.diffuse", 0);
			m_pShader->UploadInt("u_Material.specular", 1);
			m_pShader->UploadInt("u_Material.shininess", m_Shininess);
		}
	}
}