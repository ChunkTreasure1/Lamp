#include "lppch.h"
#include "Material.h"

#include "Lamp/Objects/Entity/Base/EntityManager.h"
#include "Lamp/Objects/Entity/BaseComponents/LightComponent.h"

#include "Lamp/Level/LevelSystem.h"

#include "Lamp/Rendering/Shadows/PointShadowBuffer.h"

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

		int lightCount = 0;
		/////Point lights/////
		for (auto& light : g_pEnv->pRenderUtils->GetPointLights())
		{
			if (lightCount > 11)
			{
				LP_CORE_WARN("There are more lights in scene than able to render! Will skip some lights.");
				break;
			}

			std::string v = std::to_string(lightCount);

			m_pShader->UploadFloat("u_PointLights[" + v + "].intensity", light->Intensity);
			m_pShader->UploadFloat("u_PointLights[" + v + "].radius", light->Radius);
			m_pShader->UploadFloat("u_PointLights[" + v + "].falloff", light->Falloff);
			m_pShader->UploadFloat("u_PointLights[" + v + "].farPlane", light->FarPlane);

			m_pShader->UploadFloat3("u_PointLights[" + v + "].position", light->ShadowBuffer->GetPosition());
			m_pShader->UploadFloat3("u_PointLights[" + v + "].color", light->Color);
			m_pShader->UploadInt("u_PointLights[" + v + "].shadowMap", 4 + lightCount);
				
			lightCount++;
		}
		/////////////////

		//Reserve spot 0 for shadow map
		for (int i = 4 + lightCount; i < m_pShader->GetSpecifications().TextureCount + (4 + lightCount); i++)
		{
			m_pShader->UploadInt("u_Material." + m_pShader->GetSpecifications().TextureNames[i - (4 + lightCount)], i);
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