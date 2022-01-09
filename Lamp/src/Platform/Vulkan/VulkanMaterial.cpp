#include "lppch.h"
#include "VulkanMaterial.h"

#include "Lamp/Rendering/Renderer.h"

#include "Platform/Vulkan/VulkanRenderPipeline.h"

namespace Lamp
{
	VulkanMaterial::VulkanMaterial()
		: m_index(0)
	{
	}

	VulkanMaterial::VulkanMaterial(const Ref<Material> material)
	{
		auto vulkanMaterial = std::reinterpret_pointer_cast<VulkanMaterial>(material);

		m_index = vulkanMaterial->GetIndex();
		m_name = vulkanMaterial->GetName();
		m_blendingMultiplier = vulkanMaterial->GetBlendingMultiplier();
		m_useBlending = vulkanMaterial->GetUseBlending();

		SetShader(vulkanMaterial->GetShader());
		m_textureSpecifications = vulkanMaterial->GetTextureSpecification();
	}

	VulkanMaterial::VulkanMaterial(Ref<Shader> shader, uint32_t id)
		: m_index(id)
	{
		SetShader(shader);
	}

	VulkanMaterial::VulkanMaterial(const std::string& name, Ref<Shader> shader, uint32_t id)
		: m_name(name), m_index(id)
	{
		SetShader(shader);
	}

	VulkanMaterial::VulkanMaterial(const std::string& name, uint32_t index)
		: m_index(index), m_name(name)
	{
	}

	void VulkanMaterial::Bind(Ref<RenderPipeline> renderPipeline, uint32_t currentIndex)
	{
		auto vulkanPipeline = std::reinterpret_pointer_cast<VulkanRenderPipeline>(renderPipeline);
		for (const auto& spec : m_textureSpecifications)
		{
			if (spec.texture)
			{
				vulkanPipeline->SetTexture(spec.texture, spec.binding, spec.set, currentIndex);
			}
			else
			{
				LP_CORE_ERROR("Vulkan Material: No texture bound to {0}!", spec.name);
			}
		}
	}

	void VulkanMaterial::SetTextures(const std::unordered_map<std::string, Ref<Texture2D>>& textures)
	{
		for (const auto& [name, texture] : textures)
		{
			auto texRef = FindTexture(name);
			LP_CORE_ASSERT(texRef.has_value(), "Texture does not exist in material!");

			texRef->get().texture = texture;
		}
	}

	void VulkanMaterial::SetTexture(const std::string& name, Ref<Texture2D> texture)
	{
		LP_CORE_ASSERT(texture, "Texture is nullptr!");

		auto texRef = FindTexture(name);
		LP_CORE_ASSERT(texRef.has_value(), "Texture does not exist in material!");

		texRef->get().texture = texture;
	}

	void VulkanMaterial::SetShader(Ref<Shader> shader)
	{
		m_shader = std::dynamic_pointer_cast<VulkanShader>(shader);

		m_textureSpecifications.clear();
		for (const auto& resource : m_shader->GetResources())
		{
			m_textureSpecifications.emplace_back(MaterialTextureSpecification{ resource.second.name, resource.second.set, resource.second.binding, Renderer::GetDefaultTexture() });
		}
	}

	const std::vector<Ref<Texture2D>> VulkanMaterial::GetTextures()
	{
		std::vector<Ref<Texture2D>> textures;
		for (const auto& spec : m_textureSpecifications)
		{
			textures.emplace_back(spec.texture);
		}

		return textures;
	}

	std::optional<std::reference_wrapper<VulkanMaterial::MaterialTextureSpecification>> VulkanMaterial::FindTexture(const std::string& name)
	{
		for (uint32_t i = 0; i < m_textureSpecifications.size(); i++)
		{
			if (m_textureSpecifications[i].name == name)
			{
				return m_textureSpecifications[i];
			}
		}

		return std::nullopt;
	}
}