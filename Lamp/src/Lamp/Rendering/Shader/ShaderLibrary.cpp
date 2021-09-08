#include "lppch.h"
#include "ShaderLibrary.h"

namespace Lamp
{
	std::vector<Ref<Shader>> ShaderLibrary::m_Shaders;

	void ShaderLibrary::AddShader(const std::string& path)
	{
		Ref<Shader> shader = Shader::Create(path);

		m_Shaders.push_back(shader);
	}

	void ShaderLibrary::LoadShaders()
	{
		AddShader("engine/shaders/3d/lineShader.glsl");
		AddShader("engine/shaders/3d/main/pbrForward.glsl");
		AddShader("engine/shaders/3d/ibl/eqCube.glsl");
		AddShader("engine/shaders/3d/ibl/skybox.glsl");
		AddShader("engine/shaders/3d/shadows/pointShadow.glsl");
		AddShader("engine/shaders/3d/ibl/convolution.glsl");
		AddShader("engine/shaders/3d/ibl/prefilter.glsl");
		AddShader("engine/shaders/3d/ibl/brdfIntegrate.glsl");
		AddShader("engine/shaders/3d/sprite.glsl");
		AddShader("engine/shaders/3d/main/gBuffer.glsl");
		AddShader("engine/shaders/3d/main/deferredShading.glsl");
		AddShader("engine/shaders/3d/main/ssao.glsl");
		AddShader("engine/shaders/3d/main/ssaoBlur.glsl");
		AddShader("engine/shaders/3d/selection.glsl");
		AddShader("engine/shaders/3d/shadows/dirShadow.glsl");
	}

	void ShaderLibrary::RecompileShaders()
	{
		for (auto& shader : m_Shaders)
		{
			shader->Recompile();
		}
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