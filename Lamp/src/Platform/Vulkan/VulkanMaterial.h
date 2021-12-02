
#pragma once

#include "Lamp/Rendering/Shader/Shader.h"
#include "Lamp/Mesh/Materials/Material.h"

#include "Platform/Vulkan/VulkanShader.h"

namespace Lamp
{
	class VulkanMaterial : public Material
	{
	public:
		VulkanMaterial();
		VulkanMaterial(Ref<Shader> shader, uint32_t id);
		VulkanMaterial(const std::string& name, uint32_t index);

		~VulkanMaterial() = default;

		void SetTextures(const std::unordered_map<std::string, Ref<Texture2D>>& textures) override;
		void SetTexture(const std::string& name, Ref<Texture2D> texture) override;
		
		void SetShader(Ref<Shader> shader) override;
		void SetName(const std::string& name) override { m_name = name; }
		void SetBlendingMutliplier(float value) override {}
		void SetUseBlending(bool state) override {}

		const std::unordered_map<std::string, Ref<Texture2D>>& GetTextures() override { return m_textures; }
		const uint32_t GetIndex() override { return m_index; }
		Ref<Shader> GetShader() override { return m_shader; }
		const std::string& GetName() override { return m_name; }
		const float& GetBlendingMultiplier() override { return m_blendingMultiplier; }
		const bool& GetUseBlending() override { return m_useBlending; }

	private:
		uint32_t m_index;
		std::string m_name;

		float m_blendingMultiplier = 0.f;
		bool m_useBlending = false;

		std::unordered_map<std::string, Ref<Texture2D>> m_textures;
		std::unordered_map<std::string, ShaderResourceDeclaration> m_shaderResources;

		Ref<VulkanShader> m_shader;
	};
}