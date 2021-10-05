#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	struct RenderNodeCompute : public RenderNode
	{
	public:
		virtual void Initialize() override;
		virtual void Start() override {}
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override;
		virtual RenderNodeType GetNodeType() { return RenderNodeType::Compute; }
		virtual void Serialize(YAML::Emitter& out) override;
		virtual void Deserialize(YAML::Node& node) override;

		Ref<Framebuffer> framebuffer;

	private:
		uint32_t m_WorkGroupX;
		uint32_t m_WorkGroupY;
		Ref<Shader> m_ComputeShader;
		std::vector<const char*> m_ShaderStrings;
	};
}