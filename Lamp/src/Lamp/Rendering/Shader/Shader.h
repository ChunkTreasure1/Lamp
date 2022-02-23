#pragma once

#include "Lamp/AssetSystem/Asset.h"

#include <VulkanMemoryAllocator/VulkanMemoryAllocator.h>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

#include <fstream>
#include <sstream>
#include <iostream>
#include <any>
#include <map>

namespace Lamp
{
	enum class ShaderUniformType
	{
		None = 0,
		Bool,
		Int,
		UInt,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		Int2,
		Int3,
		Int4
	};

	struct ShaderResourceDeclaration
	{
		ShaderResourceDeclaration() = default;
		ShaderResourceDeclaration(const std::string& name, uint32_t binding, uint32_t set)
			: name(name), binding(binding), set(set)
		{
		}

		std::string name;
		uint32_t binding;
		uint32_t set;
	};

	struct ShaderSpecification
	{
		std::string name;
		std::vector<std::string> inputTextureNames;
		uint32_t textureCount = 0;
		bool internalShader = true;
	};

	class Shader : public Asset
	{
	public:
		struct ShaderUniform
		{
		public:
			ShaderUniform() = default;
			ShaderUniform(std::string name, ShaderUniformType type, uint32_t size, uint32_t offset);

			const std::string& GetName() const { return m_name; }
			const uint32_t GetSize() const { return m_size; }
			const uint32_t GetOffset() const { return m_offset; }
			ShaderUniformType GetType() { return m_type; }

			static std::string UniformTypeToString(ShaderUniformType type);
		private:
			std::string m_name;
			ShaderUniformType m_type = ShaderUniformType::None;
			uint32_t m_size = 0;
			uint32_t m_offset = 0;
		};

		struct ShaderBuffer
		{
			std::string name;
			uint32_t size = 0;
			std::unordered_map<std::string, ShaderUniform> uniforms;
		};

		struct UniformBuffer
		{
			VkDescriptorBufferInfo descriptor;
			uint32_t size = 0;
			uint32_t bindPoint = 0;
			std::string name;
			VkShaderStageFlagBits shaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};

		struct StorageBuffer
		{
			VmaAllocation memoryAllocation = nullptr;
			VkDescriptorBufferInfo descriptor;
			uint32_t size = 0;
			uint32_t bindPoint = 0;
			std::string name;
			VkShaderStageFlagBits shaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};

		struct ImageSampler
		{
			uint32_t bindPoint = 0;
			uint32_t descriptorSet = 0;
			uint32_t arraySize = 0;
			std::string name;
			VkShaderStageFlagBits shaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		};

		struct PushConstantRange
		{
			VkShaderStageFlagBits shaderStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
			uint32_t offset = 0;
			uint32_t size = 0;
		};

		struct ShaderDescriptorSet
		{
			std::unordered_map<uint32_t, UniformBuffer*> uniformBuffers;
			std::unordered_map<uint32_t, StorageBuffer*> storageBuffers;
			std::unordered_map<uint32_t, ImageSampler> imageSamplers;
			std::unordered_map<uint32_t, ImageSampler> storageSamplers;

			std::unordered_map<std::string, VkWriteDescriptorSet> writeDescriptorSets;

			operator bool() const { return !(storageBuffers.empty() && uniformBuffers.empty() && imageSamplers.empty() && storageSamplers.empty()); }
		};

		struct ShaderMaterialDescriptorSet
		{
			VkDescriptorPool pool = nullptr;
			std::vector<VkDescriptorSet> descriptorSets;
		};

		Shader(const std::filesystem::path& path, bool forceCompile);
		~Shader() override;

		void Reload(bool forceCompile);
		void Bind() {}
		const std::string& GetName() { return m_specification.name; }
		const ShaderSpecification& GetSpecification() const { return m_specification; }

		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStageInfos() { return m_pipelineShaderStageInfos; }
		inline const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() { return m_descriptorSetLayouts; }
		inline const std::unordered_map<uint32_t, std::vector<VkDescriptorPoolSize>>& GetDescriptorTypes() { return m_descriptorTypes; }
		inline const std::vector<ShaderDescriptorSet>& GetDescriptorSets() { return m_shaderDescriptorSets; }
		inline const std::vector<PushConstantRange>& GetPushConstantRanges() { return m_pushConstantRanges; }
		inline const std::unordered_map<std::string, ShaderResourceDeclaration>& GetResources() { return m_resources; }
		inline const uint32_t GetDescriptorSetLayoutCount() { return static_cast<uint32_t>(m_descriptorSetLayouts.size()); }

		ShaderMaterialDescriptorSet CreateDescriptorSets(uint32_t set = 0);
		ShaderMaterialDescriptorSet CreateDescriptorSets(uint32_t set, uint32_t count);
		const VkWriteDescriptorSet* GetDescriptorSet(const std::string& name, uint32_t set = 0) const;

		VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t set);
		std::vector<VkDescriptorSetLayout> GetAllDescriptorSetLayouts();
		std::vector<VkPushConstantRange> GetAllPushConstantRanges();

		static AssetType GetStaticType() { return AssetType::Shader; }
		AssetType GetType() override { return GetStaticType(); }

		static Ref<Shader> Create(const std::filesystem::path& path, bool forceCompile = false);

	private:
		std::unordered_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& source);
		void CompileOrGetBinary(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData, bool forceCompile);
		void LoadAndCreateShaders(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData);
		void Reflect(VkShaderStageFlagBits stageFlags, const std::vector<uint32_t>& shaderData);
		void ReflectAllShaderStages(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& outShaderData);
		void CreateDescriptors();

		std::vector<ShaderDescriptorSet> m_shaderDescriptorSets;
		std::vector<PushConstantRange> m_pushConstantRanges;
		std::vector<VkPipelineShaderStageCreateInfo> m_pipelineShaderStageInfos;
		std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;

		std::unordered_map<std::string, ShaderBuffer> m_buffers;
		std::unordered_map<std::string, ShaderResourceDeclaration> m_resources;
		std::unordered_map<VkShaderStageFlagBits, std::string> m_shaderSource;
		std::unordered_map<uint32_t, std::vector<VkDescriptorPoolSize>> m_descriptorTypes;

		ShaderSpecification m_specification;
	};
}