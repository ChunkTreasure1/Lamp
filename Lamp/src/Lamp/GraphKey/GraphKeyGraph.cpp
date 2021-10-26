#include "lppch.h"
#include "GraphKeyGraph.h"

#include "NodeIncludes.h"

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
					if (attr.pLinks.size() > 0)
					{
						for (auto& link : attr.pLinks)
						{
							RemoveLink(link->id);
						}
					}
				}

				for (auto& attr : node->outputAttributes)
				{
					if (attr.pLinks.size() > 0)
					{
						for (auto& link : attr.pLinks)
						{
							RemoveLink(link->id);
						}
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
				if (attr.pLinks.size() == 0)
				{
					continue;
				}

				for (auto& link : attr.pLinks)
				{
					if (link->id == id)
					{
						if (auto it = std::find(attr.pLinks.begin(), attr.pLinks.end(), link); it != attr.pLinks.end())
						{
							attr.pLinks.erase(it);
						}
						break;
					}
				}
			}

			for (auto& attr : node->outputAttributes)
			{
				if (!attr.pLinks.size() == 0)
				{
					continue;
				}

				for (auto& link : attr.pLinks)
				{
					if (link->id == id)
					{
						if (auto it = std::find(attr.pLinks.begin(), attr.pLinks.end(), link); it != attr.pLinks.end())
						{
							attr.pLinks.erase(it);
						}
						break;
					}
				}
			}
		}

		auto func = [&id](Ref<Link>& link)
		{
			return id == link->id;
		};

		m_Specification.links.erase(std::remove_if(m_Specification.links.begin(), m_Specification.links.end(), func), m_Specification.links.end());
	}

	void GraphKeyGraph::OnEvent(Event& e)
	{
		for (auto& node : m_Specification.nodes)
		{
			node->OnEvent(e);
		}
	}
}