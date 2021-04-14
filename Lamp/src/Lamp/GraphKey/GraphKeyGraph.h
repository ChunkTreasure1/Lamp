#pragma once

#include "Node.h"
#include <rapidxml/rapidxml.hpp>

namespace Lamp
{
	struct GraphKeyGraphSpecification
	{
		std::vector<Ref<Node>> nodes;
		std::vector<Ref<Link>> links;
		std::string name;
		std::string path;
	};

	class GraphKeyGraph
	{
	public:
		GraphKeyGraph();
		GraphKeyGraph(const GraphKeyGraphSpecification& spec);

		inline GraphKeyGraphSpecification& GetSpecification() { return m_Specification; }

		void AddNode(Ref<Node> node) { m_Specification.nodes.push_back(node); }
		void AddLink(Ref<Link> link) { m_Specification.links.push_back(link); }

		void RemoveNode(uint32_t id);
		void RemoveLink(uint32_t id);

	public:
		static void Save(Ref<GraphKeyGraph>& graph, rapidxml::xml_node<>* pRoot, rapidxml::xml_document<>& doc);
		static Ref<GraphKeyGraph> Load(const std::string& path);

	private:
		GraphKeyGraphSpecification m_Specification;
	};
}