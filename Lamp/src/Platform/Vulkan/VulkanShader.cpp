#include "lppch.h"
#include "VulkanShader.h"

namespace Lamp
{
	VulkanShader::VulkanShader(const std::filesystem::path& path, bool forceCompile)
	{
	}

	VulkanShader::~VulkanShader()
	{
	}

	void VulkanShader::Reload(bool forceCompile)
	{
	}

	std::vector<VkDescriptorSetLayout> VulkanShader::GetAllDescriptorSetLayouts()
	{
		return std::vector<VkDescriptorSetLayout>();
	}

	std::vector<VkPushConstantRange> VulkanShader::GetAllPushConstantRanges()
	{
		return std::vector<VkPushConstantRange>();
	}

	std::unordered_map<VkShaderStageFlagBits, std::string> VulkanShader::PreProcess(const std::string& source)
	{
		return std::unordered_map<VkShaderStageFlagBits, std::string>();
	}

	void VulkanShader::CompileOrGetBinary(std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData, bool forceCompile)
	{
	}

	void VulkanShader::LoadAndCreateShaders(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData)
	{
	}

	void VulkanShader::Reflect(VkShaderStageFlagBits stageFlags, const std::vector<uint32_t>& shaderData)
	{
	}

	void VulkanShader::ReflectAllShaderStages(const std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData)
	{
	}

	void VulkanShader::CreateDescriptors()
	{
	}
}