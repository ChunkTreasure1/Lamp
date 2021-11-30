#pragma once

#include "Lamp/Rendering/Shader/Shader.h"
#include "VulkanAllocator.h"

namespace Lamp
{
	class VulkanShader : public Shader
	{
	public:
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

		VulkanShader(const std::filesystem::path& path, bool forceCompile);
		~VulkanShader();

		void Reload(bool forceCompile) override;

		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStageInfos() { return m_pipelineShaderStageInfos; }
		inline const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() { return m_descriptorSetLayouts; }
		inline const std::unordered_map<uint32_t, std::vector<VkDescriptorPoolSize>>& GetDescriptorTypes() { return m_descriptorTypes; }
		inline const std::vector<ShaderDescriptorSet>& GetDescriptorSets() { return m_shaderDescriptorSets; }
		inline const std::vector<PushConstantRange>& GetPushConstantRanges() { return m_pushConstantRanges; }

		std::vector<VkDescriptorSetLayout> GetAllDescriptorSetLayouts();
		std::vector<VkPushConstantRange> GetAllPushConstantRanges();

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
	};
}