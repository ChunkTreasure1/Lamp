#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	class RenderNodeEnd : public RenderNode
	{
	public:
		void Initialize() override;
		void Start() override;
		void DrawNode() override;
		void Activate(std::any value) override;
		RenderNodeType GetNodeType() override { return RenderNodeType::End; }
		void Serialize(YAML::Emitter& out) override;
		void Deserialize(YAML::Node& node) override;

		Ref<Framebuffer> framebuffer;
	};
}