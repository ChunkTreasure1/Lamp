#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	class RenderNodeCompute : public RenderNode
	{
	public:
		void Initialize() override;
		void Start() override {}
		void DrawNode() override;
		void Activate(std::any value) override;
		RenderNodeType GetNodeType() override { return RenderNodeType::Compute; }
		void Serialize(YAML::Emitter& out) override;
		void Deserialize(YAML::Node& node) override;

		Ref<Framebuffer> framebuffer;

	private:
		void SetupUniforms();
		void DrawUniforms();

	private:
		std::unordered_map<GraphUUID, std::pair<PassUniformSpecification, GraphUUID>> m_uniforms;
		Ref<RenderInputAttribute> m_runAttribute;

		uint32_t m_workGroupX;
		uint32_t m_workGroupY;
		Ref<Shader> m_computeShader;
		std::vector<const char*> m_shaderStrings;
	};
}