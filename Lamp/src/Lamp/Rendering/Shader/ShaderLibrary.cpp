#include "lppch.h"
#include "ShaderLibrary.h"

#include "Lamp/Input/FileSystem.h"

namespace Lamp
{
	std::vector<Ref<Shader>> ShaderLibrary::m_shaders;

	void ShaderLibrary::AddShader(const std::string& path)
	{
		Ref<Shader> shader = Shader::Create(path, true); //TODO: fix force compile

		m_shaders.push_back(shader);
	}

	void ShaderLibrary::LoadShaders()
	{
		std::vector<std::string> paths;
		FileSystem::GetAllFilesOfType(paths, ".glsl", "engine/shaders/vulkan");

		for (auto& path : paths)
		{
			AddShader(path);
		}
	}

	void ShaderLibrary::RecompileShaders()
	{
		for (auto& shader : m_shaders)
		{
			shader->Reload(true);
		}
	}

	Ref<Shader> ShaderLibrary::GetShader(const std::string& name)
	{
		for (auto& shader : m_shaders)
		{
			if (shader->GetName() == name)
			{
				return shader;
			}
		}

		return nullptr;
	}
}