#pragma once

#include "Node.h"

#define LP_REGISTER_NODE(x) static bool x ## _entry = Lamp::NodeRegistry::Register(x::GetFactoryName(), x::Create);

namespace Lamp
{
	class NodeRegistry
	{
	public:
		using TCreateMethod = Ref<Node>(*)();

	public:
		NodeRegistry() = delete;

		static bool Register(const std::string& name, TCreateMethod func);
		static Ref<Node> Create(const std::string& name);

		/////Start Nodes//////
		static void AddStart(Node* n);
		static void RemoveStart(Node* n);
		//////////////////////
		
		static std::unordered_map<std::string, TCreateMethod>& s_Methods();
		static std::vector<Node*>& s_StartNodes();
	};
}