#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	class RenderNodePass : public RenderNode
	{
	public:
		RenderNodePass()
		{}

		virtual ~RenderNodePass() override {}

		std::string name;
		RenderPass renderPass;

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

		void CreateDefaultRenderPipeline();

		//Drawing
		void DrawSettings();
		void DrawOutputBuffer();
		void DrawFramebuffers();

		FramebufferTextureSpecification* m_renamingAttachmentSpec = nullptr;
		FramebufferTextureSpecification* m_lastRenamingAttachmentSpec = nullptr;

		bool m_useViewportSize = false;

		std::vector<const char*> m_shaders;
		int m_bindId = 0;
	};
}