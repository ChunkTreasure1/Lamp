#include "lppch.h"
#include "ShaderPreProcessor.h"

#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "Lamp/Utility/SerializeMacros.h"

#include <yaml-cpp/yaml.h>

namespace Lamp
{
	namespace Utility
	{
		static VkShaderStageFlagBits ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")
			{
				return VK_SHADER_STAGE_VERTEX_BIT;
			}
			else if (type == "fragment" || type == "pixel")
			{
				return VK_SHADER_STAGE_FRAGMENT_BIT;
			}
			else if (type == "compute")
			{
				return VK_SHADER_STAGE_COMPUTE_BIT;
			}
			else if (type == "tessellationControl")
			{
				return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			}
			else if (type == "tessellationEvaluation")
			{
				return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			}

			return (VkShaderStageFlagBits)0;
		}
	}

	std::unordered_map<VkShaderStageFlagBits, std::string> ShaderPreProcessor::PreProcessSource(const std::string& source, const std::filesystem::path& path, ShaderMetaData& outData)
	{
		const char* beginToken = "#ShaderSpecBegin";
		const char* endToken = "#ShaderSpecEnd";

		size_t beginTokenLength = strlen(beginToken);
		std::string specificationSource;

		if (size_t beginPos = source.find(beginToken, 0); beginPos != std::string::npos)
		{
			size_t endPos = source.find(endToken, 0);
			if (endPos != std::string::npos)
			{
				size_t eol = source.find_first_of("\r\n", beginPos);
				size_t nextLinePos = source.find_first_not_of("\r\n", eol);

				specificationSource = source.substr(beginPos + (eol - beginPos) + 2, endPos - nextLinePos);
			}
			else
			{
				LP_CORE_CRITICAL("Shader {0}'s specification is corrupt! Could not preprocess!", path.string());
				LP_CORE_ASSERT(false, "Shader specification corrupt!");
			}
		}

		LP_CORE_INFO(specificationSource);

		if (!specificationSource.empty())
		{
			ProcessYAML(specificationSource, outData);
		}

		//Get sources
		std::unordered_map<VkShaderStageFlagBits, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos);
			LP_CORE_ASSERT(eol != std::string::npos, "Syntax error in shader!");

			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);

			LP_CORE_ASSERT(type == "vertex" ||
				type == "fragment" ||
				type == "pixel" ||
				type == "compute" ||
				type == "tessellationEvaluation" ||
				type == "tessellationControl", "Invalid shader type!");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);

			auto shaderType = Utility::ShaderTypeFromString(type);
			shaderSources[shaderType] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
		}

		return shaderSources;
	}

	void ShaderPreProcessor::ProcessYAML(const std::string& source, ShaderMetaData& outData)
	{
		YAML::Node root = YAML::Load(source);
		YAML::Node shaderNode = root["shader"];

		outData.name = shaderNode["name"] ? shaderNode["name"].as<std::string>() : "Null";
		outData.isInternal = shaderNode["internal"] ? shaderNode["internal"].as<bool>() : true;
	
		if (shaderNode["textures"])
		{
			YAML::Node texturesNode = shaderNode["textures"];

			for (auto entry : texturesNode)
			{
				auto& texture = outData.textureData.emplace_back();
				texture.shaderName = entry["texture"] ? entry["texture"].as<std::string>() : "Null";
				texture.editorName = entry["name"] ? entry["name"].as<std::string>() : "Null";
				texture.isInternal = entry["internal"] ? entry["internal"].as<bool>() : true;
			}
		}
	}
}