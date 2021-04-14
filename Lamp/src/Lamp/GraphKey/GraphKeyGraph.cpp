#include "lppch.h"
#include "GraphKeyGraph.h"

namespace Lamp
{
	GraphKeyGraph::GraphKeyGraph()
	{
	}

	GraphKeyGraph::GraphKeyGraph(const GraphKeyGraphSpecification& spec)
		: m_Specification(spec)
	{
	}

	void GraphKeyGraph::RemoveNode(uint32_t id)
	{
		auto func = [&id, this](Ref<Node>& node)
		{
			if (id == node->id)
			{
				for (auto& attr : node->inputAttributes)
				{
					if (attr.pLink)
					{
						RemoveLink(attr.pLink->id);
					}
				}

				for (auto& attr : node->outputAttributes)
				{
					if (attr.pLink)
					{
						RemoveLink(attr.pLink->id);
					}
				}

				return true;
			}

			return false;
		};

		m_Specification.nodes.erase(std::remove_if(m_Specification.nodes.begin(), m_Specification.nodes.end(), func), m_Specification.nodes.end());
	}
	
	void GraphKeyGraph::RemoveLink(uint32_t id)
	{
		for (auto& node : m_Specification.nodes)
		{
			for (auto& attr : node->inputAttributes)
			{
				if (!attr.pLink)
				{
					continue;
				}

				if (attr.pLink->id == id)
				{
					attr.pLink = nullptr;
					break;
				}
			}

			for (auto& attr : node->outputAttributes)
			{
				if (!attr.pLink)
				{
					continue;
				}

				if (attr.pLink->id == id)
				{
					attr.pLink = nullptr;
					break;
				}
			}
		}

		auto func = [&id](Ref<Link>& link)
		{
			return id == link->id;
		};

		m_Specification.links.erase(std::remove_if(m_Specification.links.begin(), m_Specification.links.end(), func), m_Specification.links.end());
	}

	void GraphKeyGraph::Save(Ref<GraphKeyGraph>& graph, rapidxml::xml_node<>* pRoot, rapidxml::xml_document<>& doc)
	{
		using namespace rapidxml;
		
		xml_node<>* pGraph = doc.allocate_node(node_element, "Graph");

	}

	Ref<GraphKeyGraph> GraphKeyGraph::Load(const std::string& path)
	{
		return Ref<GraphKeyGraph>();
	}
}