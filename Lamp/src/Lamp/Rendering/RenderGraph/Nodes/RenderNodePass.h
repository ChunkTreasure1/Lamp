#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	struct RenderNodePass : public RenderNode
	{
		RenderNodePass()
		{}

		virtual ~RenderNodePass() override {}

		std::string name;
		Ref<RenderPass> renderPass;

		virtual void Initialize() override;
		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override;
		virtual void Serialize(YAML::Emitter& out) override;
		virtual void Deserialize(YAML::Node& node) override;
		virtual RenderNodeType GetNodeType() { return RenderNodeType::Pass; }

	private:
		void RemoveAttribute(RenderAttributeType type, GraphUUID compId);
		void SetAttributeName(const std::string& name, GraphUUID id);
		bool IsAttributeLinked(Ref<RenderAttribute> attr);

	private:
		bool m_UseViewportSize = false;
		std::vector<const char*> m_Shaders;
		Ref<RenderInputAttribute> m_TargetBufferAttribute;
	};
}