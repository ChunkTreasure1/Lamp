#include "lppch.h"
#include "DynamicUniformRegistry.h"

namespace Lamp
{
	void DynamicUniformRegistry::AddUniform(const std::string& name, UniformType type, void* pData)
	{
		s_Uniforms().push_back(std::make_tuple(name, type, pData));
	}

	std::vector<std::tuple<std::string, UniformType, void*>>& DynamicUniformRegistry::s_Uniforms()
	{
		static std::vector<std::tuple<std::string, UniformType, void*>> impl;
		return impl;
	}
}