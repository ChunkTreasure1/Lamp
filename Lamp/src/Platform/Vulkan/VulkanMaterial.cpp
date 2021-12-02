#include "lppch.h"
#include "VulkanMaterial.h"

namespace Lamp
{
	VulkanMaterial::VulkanMaterial()
		: m_index(0)
	{
	}

	VulkanMaterial::VulkanMaterial(Ref<Shader> shader, uint32_t id)
		: m_index(id)
	{
		SetShader(shader);
	}

	VulkanMaterial::VulkanMaterial(const std::string& name, uint32_t index)
		: m_index(index), m_name(name)
	{
	}

	void VulkanMaterial::SetTextures(const std::unordered_map<std::string, Ref<Texture2D>>& textures)
	{
		for (const auto& [name, texture] : textures)
		{
			auto it = m_shaderResources.find(name);
			LP_CORE_ASSERT(it == m_shaderResources.end(), "Texture does not exist in material!");
		}

		m_textures = textures;
	}

	void VulkanMaterial::SetTexture(const std::string& name, Ref<Texture2D> texture)
	{
		auto it = m_textures.find(name);
		LP_CORE_ASSERT(it == m_textures.end(), "Texture does not exist in material!");

		it->second = texture;
	}

	void VulkanMaterial::SetShader(Ref<Shader> shader)
	{
		m_shader = std::dynamic_pointer_cast<VulkanShader>(shader);
		m_shaderResources = m_shader->GetResources();

		for (const auto& resource : m_shader->GetResources())
		{
			m_textures.emplace(resource.first, nullptr);
		}
	}
}