#pragma once

#include "Lamp/AssetSystem/Asset.h"

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <optional>

namespace Lamp
{
	class Texture2D;
	class Shader;
	class RenderPipeline;
	
	struct MaterialData
	{
		bool useBlending = false;
		bool useTranslucency = false;
		bool useAlbedo = true;
		bool useNormal = true;
		bool useMRO = true;
		bool useDetailNormal = false;

		float blendingMultiplier = 0.f;

		glm::vec2 mroColor{ 0.f, 1.f };
		glm::vec3 normalColor{ 0.f, 1.f, 0.f };
		glm::vec4 albedoColor{ 1.f, 1.f, 1.f, 1.f };
	};
	
	class Material : public Asset
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

		Material();
		Material(const Ref<Material> material);
		Material(Ref<Shader> shader, uint32_t id);
		Material(const std::string& name, Ref<Shader> shader, uint32_t id = 0);
		Material(const std::string& name, uint32_t index);

		~Material() = default;

		void Bind(Ref<RenderPipeline> renderPipeline, uint32_t currentIndex);

		void SetTextures(const std::unordered_map<std::string, Ref<Texture2D>>&textures);
		void SetTexture(const std::string & name, Ref<Texture2D> texture);

		void SetShader(Ref<Shader> shader);
		inline void SetName(const std::string & name) { m_name = name; }

		const std::vector<Ref<Texture2D>> GetTextures();
		inline const uint32_t GetIndex() { return m_index; }
		inline Ref<Shader> GetShader() { return m_shader; }

		inline const std::string& GetName() { return m_name; }
		inline std::unordered_map<uint32_t, std::vector<VkDescriptorSet>>& GetDescriptorSets() { return m_descriptorSets; }
		inline const std::vector<MaterialTextureSpecification>& GetTextureSpecification() { return m_textureSpecifications; }

		inline const MaterialData& GetMaterialData() const { return m_materialData; }

		static AssetType GetStaticType() { return AssetType::Material; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Material> Create(Ref<Shader> shader, uint32_t id);
		static Ref<Material> Create();
		static Ref<Material> Create(const Ref<Material> material);
		static Ref<Material> Create(const std::string& name, uint32_t index);

	private:
		std::optional<std::reference_wrapper<MaterialTextureSpecification>> FindTexture(const std::string& name);

		std::string m_name;
		uint32_t m_index;

		MaterialData m_materialData;

		std::vector<MaterialTextureSpecification> m_textureSpecifications;

		std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> m_descriptorSets;
		Ref<Shader> m_shader;
	};
}