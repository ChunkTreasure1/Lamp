#pragma once

#include <vector>
#include "Shader.h"
#include "Lamp/Core/Core.h"

namespace Lamp
{
	class ShaderLibrary
	{
	public:
		static void AddShader(const std::string& shader);
		static void LoadShaders();
		static void RecompileShaders();

		static Ref<Shader> GetShader(const std::string& name);
		static std::vector<Ref<Shader>>& GetShaders() { return m_shaders; }

	private:
		ShaderLibrary() = delete;

	private:
		static std::vector<Ref<Shader>> m_shaders;
	};
}