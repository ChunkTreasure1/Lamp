#include "Material.h"
#include "lppch.h"

#include "Lamp/Objects/Entity/Base/EntityManager.h"
#include "Lamp/Objects/Entity/BaseComponents/LightComponent.h"

#include "Lamp/Level/LevelSystem.h"

namespace Lamp {
void Material::SetTexture(const std::string &name, Ref<Texture2D> &texture) {
  if (m_pTextures.find(name) != m_pTextures.end()) {
    m_pTextures[name] = texture;
  }
}

void Material::SetShader(Ref<Shader> shader) {
  m_pTextures.clear();
  for (auto &name : shader->GetSpecifications().TextureNames) {
    m_pTextures.emplace(std::pair<std::string, Ref<Texture2D>>(name, nullptr));
  }

  m_pShader = shader;
}

void Material::UploadData() {
  m_pShader->Bind();

  /////Lighting/////
  m_pShader->UploadFloat3("u_DirectionalLight.diffuse",
                          g_pEnv->DirLight.Diffuse);
  m_pShader->UploadFloat3("u_DirectionalLight.specular",
                          g_pEnv->DirLight.Specular);
  m_pShader->UploadFloat3("u_DirectionalLight.direction",
                          g_pEnv->DirLight.Direction);
  m_pShader->UploadFloat3("u_DirectionalLight.position",
                          g_pEnv->DirLight.Position);

  m_pShader->UploadFloat3("u_Environment.globalAmbient",
                          LevelSystem::GetEnvironment().GlobalAmbient);

  int i = 0;
  // Point lights
  for (auto &ent : EntityManager::Get()->GetEntities()) {
    if (i > 11) {
      LP_CORE_WARN("There are more lights in scene than able to render! Will "
                   "skip some lights.");
      break;
    }

    if (auto &light = ent->GetComponent<LightComponent>()) {
      std::string val = "u_PointLight[" + std::to_string(i) + "].constant";

      m_pShader->UploadFloat("u_PointLight[" + std::to_string(i) + "].constant",
                             light->GetLightConstant());
      m_pShader->UploadFloat("u_PointLight[" + std::to_string(i) + "].linear",
                             light->GetLinearConstant());
      m_pShader->UploadFloat("u_PointLight[" + std::to_string(i) +
                                 "].quadratic",
                             light->GetQuadraticConstant());

      m_pShader->UploadFloat3("u_PointLight[" + std::to_string(i) +
                                  "].position",
                              light->GetOwner()->GetPosition());
      m_pShader->UploadFloat3("u_PointLight[" + std::to_string(i) + "].diffuse",
                              light->GetDiffuse());
      m_pShader->UploadFloat3("u_PointLight[" + std::to_string(i) +
                                  "].specular",
                              light->GetSpecular());
      i++;
    }
  }
  /////////////////

  m_pShader->UploadInt("u_Material.diffuse", 0);
  m_pShader->UploadInt("u_Material.specular", 1);
  m_pShader->UploadFloat("u_Material.shininess", m_Shininess);
  m_pShader->UploadInt("u_LightCount", i);
}
} // namespace Lamp