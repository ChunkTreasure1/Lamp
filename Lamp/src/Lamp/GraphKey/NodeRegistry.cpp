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

	void NodeRegistry::AddStart(Node* n)
	{
		s_StartNodes().push_back(n);
	}

	void NodeRegistry::RemoveStart(Node* n)
	{
		if (auto it = std::find(s_StartNodes().begin(), s_StartNodes().end(), n); it != s_StartNodes().end())
		{
			s_StartNodes().erase(it);
		}
	}

	std::unordered_map<std::string, NodeRegistry::TCreateMethod>& NodeRegistry::s_Methods()
	{
		static std::unordered_map<std::string, NodeRegistry::TCreateMethod> impl;
		return impl;
	}

	std::vector<Node*>& NodeRegistry::s_StartNodes()
	{
		static std::vector<Node*> impl;
		return impl;
	}
}