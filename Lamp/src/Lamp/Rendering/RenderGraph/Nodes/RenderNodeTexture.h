#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	struct RenderNodeTexture : public RenderNode
	{
		RenderNodeTexture()
		{
		}

		virtual void Initialize() override;
		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override {}
		virtual RenderNodeType GetNodeType() { return RenderNodeType::Texture; }
		virtual void Serialize(YAML::Emitter& out) override;
		virtual void Deserialize(YAML::Node& node) override;

		Ref<Texture2D> texture;

	private:
		void GetTexture();
	};
}