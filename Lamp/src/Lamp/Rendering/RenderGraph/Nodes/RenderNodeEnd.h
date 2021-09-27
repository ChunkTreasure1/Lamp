#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	class RenderNodeEnd : public RenderNode
	{
	public:
		virtual void Initialize() override;
		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override;
		virtual RenderNodeType GetNodeType() { return RenderNodeType::End; }
		virtual void Serialize(YAML::Emitter& out) override;
		virtual void Deserialize(YAML::Node& node) override;

		Ref<Framebuffer> framebuffer;
	};
}