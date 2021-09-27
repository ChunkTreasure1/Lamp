#pragma once

#include <unordered_map>
#include "Lamp/Rendering/Shader/Shader.h"

namespace Lamp
{
	class DynamicUniformRegistry
	{
	public:
		static void AddUniform(const std::string& name, UniformType type, void* pData);

		static std::vector<std::tuple<std::string, UniformType, void*>>& s_Uniforms(); //name, type, data
	};
}