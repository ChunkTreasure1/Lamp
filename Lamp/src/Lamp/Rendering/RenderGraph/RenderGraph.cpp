#include "lppch.h"
#include "RenderGraph.h"

#include "Lamp/Rendering/RenderGraph/Nodes/RenderNodeStart.h"

#include <yaml-cpp/yaml.h>

namespace Lamp
{
	RenderGraph::RenderGraph(const RenderGraphSpecification& spec)
		: m_specification(spec)
	{
	}

	void RenderGraph::AddNode(Ref<RenderNode> node)
	{
		m_specification.nodes.push_back(node);
	}

	void RenderGraph::AddLink(Ref<RenderLink> link)
	{
		m_specification.links.push_back(link);
	}

	void RenderGraph::RemoveNode(GraphUUID id)
	{
		auto func = [&id, this](Ref<RenderNode>& node)
		{
			if (id == node->id)
			{
				for (int i = node->links.size() - 1; i >= 0; i--)
				{
					RemoveLink(node->links[i]->id);
				}

				node->links.clear();

				if (node->GetNodeType() == RenderNodeType::Start)
				{
					m_specification.startNode = nullptr;
				}
				else if (node->GetNodeType() == RenderNodeType::End)
				{
					m_specification.endNode = nullptr;
				}
				return true;
			}
			return false;
		};
		m_specification.nodes.erase(std::remove_if(m_specification.nodes.begin(), m_specification.nodes.end(), func), m_specification.nodes.end());
	}

	void RenderGraph::RemoveLink(GraphUUID id)
	{
		auto func = [&id](Ref<RenderLink>& link)
		{
			return id == link->id;
		};

		for (auto& node : m_specification.nodes)
		{
			node->links.erase(std::remove_if(node->links.begin(), node->links.end(), func), node->links.end());
		}

		m_specification.links.erase(std::remove_if(m_specification.links.begin(), m_specification.links.end(), func), m_specification.links.end());
	}

	void RenderGraph::Run(Ref<CameraBase> camera)
	{
		LP_PROFILE_FUNCTION();
		if (m_specification.startNode)
		{
			m_specification.startNode->Activate(camera);
		}
	}

	void RenderGraph::Start()
	{
		for (const auto& node : m_specification.nodes)
		{
			node->Start();
		}
	}
}