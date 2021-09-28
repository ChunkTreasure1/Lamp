#pragma once

#include "RenderNode.h"
#include "Lamp/AssetSystem/Asset.h"
namespace Lamp
{
	struct RenderNodeEnd;
	struct RenderNodeStart;

	struct RenderGraphSpecification
	{
		std::vector<Ref<Lamp::RenderLink>> links;
		std::vector<Ref<Lamp::RenderNode>> nodes;

		Ref<RenderNodeEnd> endNode;
		Ref<RenderNodeStart> startNode;

		std::string name;
	};

	class RenderGraph : public Asset
	{
	public:
		RenderGraph();
		RenderGraph(const RenderGraphSpecification& spec);

		~RenderGraph() = default;

		inline RenderGraphSpecification& GetSpecification() { return m_Specification; }

		void AddNode(Ref<RenderNode> node);
		void AddLink(Ref<RenderLink> link);

		void RemoveNode(GraphUUID id);
		void RemoveLink(GraphUUID id);

		void Run(Ref<CameraBase> camera);
		void Start();

		static AssetType GetStaticType() { return AssetType::RenderGraph; }
		virtual AssetType GetType() override { return GetStaticType(); }

	private:
		RenderGraphSpecification m_Specification;
	};
}