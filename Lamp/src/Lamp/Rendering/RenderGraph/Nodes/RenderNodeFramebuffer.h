#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	struct RenderNodeFramebuffer : public RenderNode
	{
		RenderNodeFramebuffer()
		{}

		virtual ~RenderNodeFramebuffer() override {}

		Ref<Framebuffer> framebuffer;

		virtual void Initialize() override;
		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override {}
		virtual RenderNodeType GetNodeType() { return RenderNodeType::Framebuffer; }
		virtual void Serialize(YAML::Emitter& node) override;
		virtual void Deserialize(YAML::Node& node) override;
	
	private:
		bool m_UseInternalBuffers = false;
		int m_CurrentlySelectedBuffer = 0;
		std::string m_SelectedBufferName = "";
		std::vector<const char *> m_BufferNames;
	};
}