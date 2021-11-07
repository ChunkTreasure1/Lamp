#include "lppch.h"
#include "RenderGraph.h"

#include "Nodes/RenderNodeStart.h"

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
	}

	void RenderGraph::AddLink(Ref<RenderLink> link)
	{
		m_Specification.links.push_back(link);
	}

	void RenderGraph::RemoveNode(GraphUUID id)
	{
		auto func = [&id, this](Ref<RenderNode>& node)
		{
			if (id == node->id)
			{
				for (const auto& link : node->links)
				{
					RemoveLink(link->id);
				}

				node->links.clear();

				if (node->GetNodeType() == RenderNodeType::Start)
				{
					m_Specification.startNode = nullptr;
				}
				else if (node->GetNodeType() == RenderNodeType::End)
				{
					m_Specification.endNode = nullptr;
				}
				return true;
			}
			return false;
		};
		m_Specification.nodes.erase(std::remove_if(m_Specification.nodes.begin(), m_Specification.nodes.end(), func), m_Specification.nodes.end());
	}

	void RenderGraph::RemoveLink(GraphUUID id)
	{
		auto func = [&id](Ref<RenderLink>& link)
		{
			return id == link->id;
		};

		for (auto& node : m_Specification.nodes)
		{
			node->links.erase(std::remove_if(node->links.begin(), node->links.end(), func), node->links.end());
		}

		m_Specification.links.erase(std::remove_if(m_Specification.links.begin(), m_Specification.links.end(), func), m_Specification.links.end());
	}

	void RenderGraph::Run(Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();
		if (m_Specification.startNode)
		{
			m_Specification.startNode->Activate(camera);
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