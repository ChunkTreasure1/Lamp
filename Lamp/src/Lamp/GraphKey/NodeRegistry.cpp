#include "lppch.h"
#include "NodeRegistry.h"

namespace Lamp
{
	bool NodeRegistry::Register(const std::string& name, TCreateMethod func)
	{
		if (auto it = s_Methods().find(name); it == s_Methods().end())
		{
			s_Methods()[name] = func;
			return true;
		}

		return false;
	}
	Ref<Node> NodeRegistry::Create(const std::string& name)
	{
		if (auto it = s_Methods().find(name); it != s_Methods().end())
		{
			return it->second();
		}

		return nullptr;
	}
	std::unordered_map<std::string, NodeRegistry::TCreateMethod>& NodeRegistry::s_Methods()
	{
		static std::unordered_map<std::string, NodeRegistry::TCreateMethod> impl;
		return impl;
	}
}