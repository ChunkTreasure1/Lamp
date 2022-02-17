#include "lppch.h"
#include "Material.h"

#include "Lamp/Rendering/Shader/Shader.h"
#include "Lamp/Rendering/RenderPipeline.h"

#include "Platform/Vulkan/VulkanRenderer.h"

namespace Lamp
{
	Ref<Material> Material::Create(Ref<Shader> shader, uint32_t id)
	{
		return CreateRef<Material>(shader, id);
	}

	Ref<Material> Material::Create()
	{
		return CreateRef<Material>();
	}

	Ref<Material> Material::Create(const Ref<Material> material)
	{
		return CreateRef<Material>(material);
	}

	Ref<Material> Material::Create(const std::string& name, uint32_t index)
	{
		return CreateRef<Material>(name, index);
	}

	std::optional<std::reference_wrapper<Material::MaterialTextureSpecification>> Material::FindTexture(const std::string& name)
	{
		for (auto& textureSpecification : m_textureSpecifications)
		{
			if (textureSpecification.name == name)
			{
				return textureSpecification;
			}
		}

		return std::nullopt;
	}

	Material::Material()
	{
	}

	Material::Material(const Ref<Material> material)
	{
		m_index = material->GetIndex();
		m_name = material->GetName();
		m_materialData = material->GetMaterialData();

		SetShader(material->GetShader());
		m_textureSpecifications = material->GetTextureSpecification();
	}

	Material::Material(Ref<Shader> shader, uint32_t id)
		: m_index(id)
	{
		SetShader(shader);
	}

	Material::Material(const std::string& name, Ref<Shader> shader, uint32_t id)
		: m_name(name), m_index(id)
	{
		SetShader(shader);
	}

	Material::Material(const std::string& name, uint32_t index)
		: m_index(index), m_name(name)
	{
	}

	void Material::Bind(Ref<RenderPipeline> renderPipeline, uint32_t currentIndex)
	{
		for (const auto& spec : m_textureSpecifications)
		{
			if (spec.texture)
			{
				renderPipeline->SetTexture(spec.texture, spec.binding, spec.set, currentIndex);
			}
			else
			{
				LP_CORE_ERROR("Vulkan Material: No texture bound to {0}!", spec.name);
			}
		}
	}

	void Material::SetTextures(const std::unordered_map<std::string, Ref<Texture2D>>& textures)
	{
		for (const auto& [name, texture] : textures)
		{
			auto texRef = FindTexture(name);
			LP_CORE_ASSERT(texRef.has_value(), "Texture does not exist in material!");

			texRef->get().texture = texture;
		}
	}

	void Material::SetTexture(const std::string& name, Ref<Texture2D> texture)
	{
		LP_CORE_ASSERT(texture, "Texture is nullptr!");

		auto texRef = FindTexture(name);
		LP_CORE_ASSERT(texRef.has_value(), "Texture does not exist in material!");

		texRef->get().texture = texture;
	}

	void Material::SetShader(Ref<Shader> shader)
	{
		m_shader = shader;

		m_textureSpecifications.clear();
		for (const auto& resource : shader->GetResources())
		{
			m_textureSpecifications.emplace_back(MaterialTextureSpecification{ resource.second.name, resource.second.set, resource.second.binding, Renderer::Get().GetDefaults().whiteTexture });
		}
	}

	const std::vector<Ref<Texture2D>> Material::GetTextures()
	{
		std::vector<Ref<Texture2D>> textures;
		for (const auto& spec : m_textureSpecifications)
		{
			textures.emplace_back(spec.texture);
		}

		return textures;
	}
}