#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	struct RenderNodePass : public RenderNode
	{
		RenderNodePass()
		{
		}

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
		std::vector<const char*> m_Shaders;
	};
}