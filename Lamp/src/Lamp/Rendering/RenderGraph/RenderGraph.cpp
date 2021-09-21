#include "lppch.h"
#include "RenderGraph.h"

namespace Lamp
{
	RenderGraph::RenderGraph()
	{
	}

	RenderGraph::RenderGraph(const RenderGraphSpecification& spec)
		: m_Specification(spec)
	{
	}

	void RenderGraph::AddNode(Ref<RenderNode> node)
	{
		m_Specification.nodes.push_back(node);
		node->id = m_CurrentId++;
	}

	void RenderGraph::AddLink(Ref<RenderLink> link)
	{
		m_Specification.links.push_back(link);
		link->id = m_CurrentId++;
	}

	void RenderGraph::RemoveNode(uint32_t id)
	{
	}

	void RenderGraph::RemoveLink(uint32_t id)
	{
	}
}