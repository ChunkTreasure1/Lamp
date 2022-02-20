
#pragma once

#include "Lamp/Rendering/Shader/Shader.h"
#include "Lamp/Mesh/Materials/Material.h"

#include "Platform/Vulkan/VulkanShader.h"

#include <optional>

namespace Lamp
{
	class VulkanMaterial : public Material
	{
	public:
		struct MaterialTextureSpecification
		{
			MaterialTextureSpecification() = default;
			MaterialTextureSpecification(const std::string& aName, uint32_t aSet, uint32_t aBinding, Ref<Texture2D> aTexture)
				: name(aName), set(aSet), binding(aBinding), texture(aTexture)
			{
			}

			std::string name;
			uint32_t set;
			uint32_t binding;
			Ref<Texture2D> texture;
		};

		VulkanMaterial();
		VulkanMaterial(const Ref<Material> material);
		VulkanMaterial(Ref<Shader> shader, uint32_t id);
		VulkanMaterial(const std::string& name, Ref<Shader> shader, uint32_t id = 0);
		VulkanMaterial(const std::string& name, uint32_t index);

		~VulkanMaterial() = default;

		void Bind(Ref<RenderPipeline> renderPipeline, uint32_t currentIndex) override;

		void SetTextures(const std::unordered_map<std::string, Ref<Texture2D>>& textures) override;
		void SetTexture(const std::string& name, Ref<Texture2D> texture) override;
		
		void SetShader(Ref<Shader> shader) override;
		inline void SetName(const std::string& name) override { m_name = name; }

		const std::vector<Ref<Texture2D>> GetTextures() override;
		inline const uint32_t GetIndex() override { return m_index; }
		inline Ref<Shader> GetShader() override { return m_shader; }
		
		inline const std::string& GetName() override { return m_name; }
		inline std::unordered_map<uint32_t, std::vector<VkDescriptorSet>>& GetDescriptorSets() { return m_descriptorSets; }
		inline const std::vector<MaterialTextureSpecification>& GetTextureSpecification() { return m_textureSpecifications; }

		inline const MaterialData& GetMaterialData() const override { return m_materialData; }

	private:
		std::optional<std::reference_wrapper<MaterialTextureSpecification>> FindTexture(const std::string& name);

		std::string m_name;
		uint32_t m_index;

		MaterialData m_materialData;

		std::vector<MaterialTextureSpecification> m_textureSpecifications;

		std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> m_descriptorSets;
		Ref<VulkanShader> m_shader;
	};
}