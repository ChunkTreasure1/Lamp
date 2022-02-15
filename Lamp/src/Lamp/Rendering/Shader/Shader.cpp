#include "lppch.h"
#include "Shader.h"

#include "Platform/Vulkan/VulkanShader.h"

namespace Lamp
{
	Ref<Shader> Shader::Create(const std::filesystem::path& path, bool forceCompile)
	{
		return CreateRef<VulkanShader>(path, forceCompile);
	}

	Shader::ShaderUniform::ShaderUniform(std::string name, ShaderUniformType type, uint32_t size, uint32_t offset)
		: m_name(name), m_type(type), m_size(size), m_offset(offset)
	{
	}

	std::string Shader::ShaderUniform::UniformTypeToString(ShaderUniformType type)
	{
		switch (type)
		{
			case ShaderUniformType::None: return "None";
			case ShaderUniformType::Bool: return "Bool";
			case ShaderUniformType::Int: return "Int";
			case ShaderUniformType::UInt: return "UInt";
			case ShaderUniformType::Float: return "Float";
			case ShaderUniformType::Float2: return "Float2";
			case ShaderUniformType::Float3: return "Float3";
			case ShaderUniformType::Float4: return "Float4";
			case ShaderUniformType::Mat3: return "Mat3";
			case ShaderUniformType::Mat4: return "Mat4";
			case ShaderUniformType::Int2: return "Int2";
			case ShaderUniformType::Int3: return "Int3";
			case ShaderUniformType::Int4: return "Int4";
		}

		return "None";
	}
}