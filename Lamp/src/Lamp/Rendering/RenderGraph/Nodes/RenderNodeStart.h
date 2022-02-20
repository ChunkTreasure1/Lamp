#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	class RenderNodeStart : public RenderNode
	{
	public:
		RenderNodeStart() = default;
		~RenderNodeStart() override = default;

		void Initialize() override;
		void Start() override;
		void DrawNode() override;
		void Activate(std::any value) override;
		RenderNodeType GetNodeType() override;
		void Serialize(YAML::Emitter& out) override;
		void Deserialize(YAML::Node& node) override;
	};
}