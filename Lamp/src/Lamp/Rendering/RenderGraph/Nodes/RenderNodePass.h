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

		void Initialize() override;
		void Start() override;
		void DrawNode() override;
		void Activate(std::any value) override;
		void Serialize(YAML::Emitter& out) override;
		void Deserialize(YAML::Node& node) override;
		RenderNodeType GetNodeType() override { return RenderNodeType::Pass; }

	private:
		void RemoveAttribute(RenderAttributeType type, GraphUUID compId);
		void SetAttributeName(const std::string& name, GraphUUID id);
		bool IsAttributeLinked(Ref<RenderAttribute> attr);

		//Drawing
		void DrawUniforms();

	private:
		bool m_UseViewportSize = false;

		std::vector<const char*> m_Shaders;
		Ref<RenderInputAttribute> m_TargetBufferAttribute;
	};
}