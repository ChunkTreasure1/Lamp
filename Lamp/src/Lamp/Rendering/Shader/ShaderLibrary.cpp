#include "lppch.h"
#include "ShaderLibrary.h"

namespace Lamp
{
	std::vector<Ref<Shader>> ShaderLibrary::m_Shaders;

	void ShaderLibrary::AddShader(const std::string& vertexPath, const std::string& fragmentPath)
	{
		Ref<Shader> shader = Shader::Create(vertexPath, fragmentPath);

		m_Shaders.push_back(shader);
	}

	void ShaderLibrary::LoadShaders()
	{
		AddShader("engine/shaders/3d/lineShader_vs.glsl", "engine/shaders/3d/lineShader_fs.glsl");
		AddShader("engine/shaders/3d/shader_vs.glsl", "engine/shaders/3d/shader_fs.glsl");
		AddShader("engine/shaders/3d/testPbr_vs.glsl", "engine/shaders/3d/testPbr_fs.glsl");
		AddShader("engine/shaders/3d/eqCube_vs.glsl", "engine/shaders/3d/eqCube_fs.glsl");
		AddShader("engine/shaders/3d/skybox_vs.glsl", "engine/shaders/3d/skybox_fs.glsl");
	}

	Ref<Shader>& ShaderLibrary::GetShader(const std::string& name)
	{
		for (auto& shader : m_Shaders)
		{
			if (shader->GetName() == name)
			{
				return shader;
			}
		}

		return Ref<Shader>(nullptr);
	}
}