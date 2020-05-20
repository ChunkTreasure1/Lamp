#include "lppch.h"
#include "ComponentRegistry.h"

namespace Lamp
{
	bool ComponentRegistry::Register(const std::string& name, TCreateMethod func)
	{
		if (auto it = s_Methods().find(name); it == s_Methods().end())
		{
			s_Methods()[name] = func;
			return true;
		}

		return false;
	}

	Ref<EntityComponent> ComponentRegistry::Create(const std::string& name)
	{
		if (auto it = s_Methods().find(name); it != s_Methods().end())
		{
			return it->second();
		}

		return nullptr;
	}

	std::unordered_map<std::string, ComponentRegistry::TCreateMethod>& ComponentRegistry::s_Methods()
	{
		static std::unordered_map<std::string, ComponentRegistry::TCreateMethod> impl;
		return impl;
	}

}