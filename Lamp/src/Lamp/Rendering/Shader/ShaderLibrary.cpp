#include "lppch.h"
#include "ShaderLibrary.h"

namespace Lamp
{
	std::vector<Ref<Shader>> ShaderLibrary::m_Shaders;

	void ShaderLibrary::AddShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geoPath)
	{
		Ref<Shader> shader = Shader::Create({ vertexPath, fragmentPath});

		m_Shaders.push_back(shader);
	}

	void ShaderLibrary::LoadShaders()
	{
		AddShader("engine/shaders/3d/lineShader_vs.glsl", "engine/shaders/3d/lineShader_fs.glsl");
		AddShader("engine/shaders/3d/shader_vs.glsl", "engine/shaders/3d/shader_fs.glsl");
		AddShader("engine/shaders/3d/testPbr_vs.glsl", "engine/shaders/3d/testPbr_fs.glsl");
		AddShader("engine/shaders/3d/ibl/eqCube_vs.glsl", "engine/shaders/3d/ibl/eqCube_fs.glsl");
		AddShader("engine/shaders/3d/ibl/skybox_vs.glsl", "engine/shaders/3d/ibl/skybox_fs.glsl");
		AddShader("engine/shaders/3d/shadows/dirShadow_vs.glsl", "engine/shaders/3d/shadows/dirShadow_fs.glsl");
		AddShader("engine/shaders/3d/shadows/pointShadow_vs.glsl", "engine/shaders/3d/shadows/pointShadow_fs.glsl", "engine/shaders/3d/shadows/pointShadow_gs.glsl");
		AddShader("engine/shaders/3d/ibl/eqCube_vs.glsl", "engine/shaders/3d/ibl/convolution_fs.glsl");
		AddShader("engine/shaders/3d/ibl/eqCube_vs.glsl", "engine/shaders/3d/ibl/prefilter_fs.glsl");
		AddShader("engine/shaders/3d/ibl/brdfIntegrate_vs.glsl", "engine/shaders/3d/ibl/brdfIntegrate_fs.glsl");
		AddShader("engine/shaders/3d/sprite_vs.glsl", "engine/shaders/3d/sprite_fs.glsl");
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