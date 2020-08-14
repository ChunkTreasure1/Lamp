#include "lppch.h"
#include "Material.h"

#include "Lamp/Objects/Entity/Base/EntityManager.h"
#include "Lamp/Objects/Entity/BaseComponents/LightComponent.h"

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
			
			int i = 0;
			//Point lights
			for (auto& ent : EntityManager::Get()->GetEntities())
			{
				if (i > 11)
				{
					break;
				}

				if (auto& light = ent->GetComponent<LightComponent>())
				{
					std::string val = "u_PointLight[" + std::to_string(i) + "].constant";

					m_pShader->UploadFloat("u_PointLight[" + std::to_string(i) + "].constant", light->GetLightConstant());
					m_pShader->UploadFloat("u_PointLight[" + std::to_string(i) + "].linear", light->GetLinearConstant());
					m_pShader->UploadFloat("u_PointLight[" + std::to_string(i) + "].quadratic", light->GetQuadraticConstant());

					m_pShader->UploadFloat3("u_PointLight[" + std::to_string(i) + "].position", light->GetOwner()->GetPosition());
					m_pShader->UploadFloat3("u_PointLight[" + std::to_string(i) + "].ambient", light->GetAmbient());
					m_pShader->UploadFloat3("u_PointLight[" + std::to_string(i) + "].diffuse", light->GetDiffuse());
					m_pShader->UploadFloat3("u_PointLight[" + std::to_string(i) + "].specular", light->GetSpecular());
					i++;
				}
			}
			/////////////////

			m_pShader->UploadInt("u_Material.diffuse", 0);
			m_pShader->UploadInt("u_Material.specular", 1);
			m_pShader->UploadFloat("u_Material.shininess", m_Shininess);
			m_pShader->UploadInt("u_LightCount", i);
		}
	}
}