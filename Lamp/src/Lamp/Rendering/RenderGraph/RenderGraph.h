#pragma once

#include "RenderNode.h"
#include "Lamp/AssetSystem/Asset.h"
namespace Lamp
{
	struct RenderGraphSpecification
	{
		std::vector<Ref<Lamp::RenderLink>> links;
		std::vector<Ref<Lamp::RenderNode>> nodes;
		std::string name;
	};

	class RenderGraph : public Asset
	{
	public:
		RenderGraph();
		RenderGraph(const RenderGraphSpecification& spec);

		~RenderGraph() = default;

		inline RenderGraphSpecification& GetSpecification() { return m_Specification; }
		inline uint32_t& GetCurrentId() { return m_CurrentId; }
		inline void SetCurrentId(uint32_t id) { m_CurrentId = id; }

		void AddNode(Ref<RenderNode> node);
		void AddLink(Ref<RenderLink> link);

		void RemoveNode(uint32_t id);
		void RemoveLink(uint32_t id);

		static AssetType GetStaticType() { return AssetType::RenderGraph; }
		virtual AssetType GetType() override { return GetStaticType(); }

	private:
		RenderGraphSpecification m_Specification;
		uint32_t m_CurrentId = 0;
	};
}