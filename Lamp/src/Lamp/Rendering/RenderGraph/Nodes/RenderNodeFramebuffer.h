#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	class RenderNodeFramebuffer : public RenderNode
	{
	public:
		RenderNodeFramebuffer()
		{}

		virtual ~RenderNodeFramebuffer() override {}

		Ref<Framebuffer> framebuffer;

		void Initialize() override;
		void Start() override;
		void DrawNode() override;
		void Activate(std::any value) override {}
		RenderNodeType GetNodeType() override { return RenderNodeType::Framebuffer; }
		void Serialize(YAML::Emitter& node) override;
		void Deserialize(YAML::Node& node) override;
	
	private:
		bool m_useInternalBuffers = false;
		bool m_useScreenSize = false;

		int m_bindId = 0;

		int m_CurrentlySelectedBuffer = 0;
		std::string m_SelectedBufferName;
		std::vector<const char *> m_BufferNames;
	};
}