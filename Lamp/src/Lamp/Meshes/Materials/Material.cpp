#include "lppch.h"
#include "Material.h"

#include "Lamp/Objects/Entity/Base/EntityManager.h"
#include "Lamp/Objects/Entity/BaseComponents/LightComponent.h"

#include "Lamp/Level/LevelSystem.h"

namespace Lamp
{
	void Material::SetTexture(const std::string& name, Ref<Texture2D>& texture)
	{
		if (m_pTextures.find(name) != m_pTextures.end())
		{
			m_pTextures[name] = texture;
		}
	}

	void Material::SetShader(Ref<Shader> shader)
	{
		m_pTextures.clear();
		for (auto& name : shader->GetSpecifications().TextureNames)
		{
			m_pTextures.emplace(std::pair<std::string, Ref<Texture2D>>(name, nullptr));
		}

		m_pShader = shader;
	}

	void Material::UploadData()
	{
		m_pShader->Bind();

		/////Lighting/////
		m_pShader->UploadFloat("u_DirectionalLight.intensity", g_pEnv->DirLight.Intensity);
		m_pShader->UploadFloat3("u_DirectionalLight.direction", glm::normalize(g_pEnv->DirLight.Position));
		m_pShader->UploadFloat3("u_DirectionalLight.color", g_pEnv->DirLight.Color);

		//m_pShader->UploadFloat3("u_Environment.globalAmbient", LevelSystem::GetEnvironment().GlobalAmbient);

		int lightCount = 0;
		//Point lights
		for (auto& ent : EntityManager::Get()->GetEntities())
		{
			if (lightCount > 11)
			{
				LP_CORE_WARN("There are more lights in scene than able to render! Will skip some lights.");
				break;
			}

			if (auto& light = ent->GetComponent<LightComponent>())
			{
				m_pShader->UploadFloat("u_PointLights[" + std::to_string(lightCount) + "].intensity", light->GetIntensity());
				m_pShader->UploadFloat("u_PointLights[" + std::to_string(lightCount) + "].radius", light->GetRadius());
				m_pShader->UploadFloat("u_PointLights[" + std::to_string(lightCount) + "].falloff", light->GetFalloff());

				m_pShader->UploadFloat3("u_PointLights[" + std::to_string(lightCount) + "].position", light->GetOwner()->GetPosition());
				m_pShader->UploadFloat3("u_PointLights[" + std::to_string(lightCount) + "].color", light->GetColor());
				lightCount++;
			}
		}
		/////////////////

		//Reserve spot 0 for shadow map
		for (int i = 1; i <= m_pShader->GetSpecifications().TextureCount; i++)
		{
			m_pShader->UploadInt("u_Material." + m_pShader->GetSpecifications().TextureNames[i - 1], i);
		}

			//m_pShader->UploadFloat("u_Material.depthScale", m_DepthScale);
		m_pShader->UploadInt("u_LightCount", lightCount);

			//if (m_pTextures.find("Depth") != m_pTextures.end())
			//{
			//	if (m_pTextures["Depth"].get())
			//	{
			//		m_pShader->UploadInt("u_UsingParallax", 1);
			//	}
			//}
	}
}