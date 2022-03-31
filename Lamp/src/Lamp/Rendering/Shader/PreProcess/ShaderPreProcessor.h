#pragma once

#include <vulkan/vulkan.h>

#include <unordered_map>

namespace Lamp
{
	struct ShaderTextureMetaData
	{
		std::string shaderName;
		std::string editorName;
		bool isInternal;
	};
	
	struct ShaderMetaData
	{
		std::string name = "Null";
		bool isInternal = true;
	
		std::vector<ShaderTextureMetaData> textureData;
	};
	
	class ShaderPreProcessor
	{
	public:
		static std::unordered_map<VkShaderStageFlagBits, std::string> PreProcessSource(const std::string& source, const std::filesystem::path& path, ShaderMetaData& outData);
		
	private:
		static void ProcessYAML(const std::string& source, ShaderMetaData& outData);
	};
}