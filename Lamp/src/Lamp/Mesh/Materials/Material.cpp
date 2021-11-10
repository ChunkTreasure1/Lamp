#include "lppch.h"
#include "Material.h"

#include "Lamp/AssetSystem/ResourceCache.h"
#include "Lamp/Rendering/Shader/Shader.h"

namespace Lamp
{
	void Material::SetTexture(const std::string& name, Ref<Texture2D> texture)
	{
		if (m_pTextures.find(name) != m_pTextures.end())
		{
			m_pTextures[name] = texture;
		}
	}

	void Material::SetShader(Ref<Shader> shader)
	{
		m_pTextures.clear();
		for (auto& name : shader->GetSpecification().textureNames)
		{
			m_pTextures.emplace(name, ResourceCache::GetAsset<Texture2D>("engine/textures/default/defaultTexture.png"));
		}

		m_pShader = shader;
	}
}