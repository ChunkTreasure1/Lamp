#include "lppch.h"
#include "RenderGraph.h"

#include <yaml-cpp/yaml.h>

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

	void RenderGraph::Run(Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();
		for (const auto& node : m_Specification.startNodes)
		{
			node->Activate(camera);
		}
	}

	void RenderGraph::Start()
	{
		for (const auto& node : m_Specification.nodes)
		{
			node->Start();
		}
	}
}