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
		inline uint32_t GetCurrentId() { return m_CurrentId; }
		inline void SetCurrentId(uint32_t id) { m_CurrentId = id; }

		void AddNode(Ref<Node> node) { m_Specification.nodes.push_back(node); }
		void AddLink(Ref<Link> link) { m_Specification.links.push_back(link); }

		void RemoveNode(uint32_t id);
		void RemoveLink(uint32_t id);

	public:
		static void Save(Ref<GraphKeyGraph>& graph, rapidxml::xml_node<>* pRoot, rapidxml::xml_document<>& doc);
		static void Load(Ref<GraphKeyGraph>& graph, rapidxml::xml_node<>* pRoot);

	private:
		static void AppendAttribute(Attribute& attr, const std::string& type, rapidxml::xml_node<>* pRoot, rapidxml::xml_document<>& doc);
		static void LoadAttribute(Attribute& attr, rapidxml::xml_node<>* pRoot, const std::string& name);

	private:
		GraphKeyGraphSpecification m_Specification;
		uint32_t m_CurrentId = 0;
	};
}