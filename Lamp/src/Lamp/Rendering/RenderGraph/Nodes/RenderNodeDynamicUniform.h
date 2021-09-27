#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	struct RenderNodeDynamicUniform : public RenderNode
	{
		virtual void Initialize() override;
		virtual void Start() override;
		virtual void DrawNode() override;
		virtual void Activate(std::any value) override {}
		virtual RenderNodeType GetNodeType() { return RenderNodeType::DynamicUniform; }
		virtual void Serialize(YAML::Emitter& node) override;
		virtual void Deserialize(YAML::Node& node) override;

		void* pData = nullptr;
		UniformType uniformType;
		std::string dataName;

	private:
		std::vector<const char*> m_Uniforms;
		int m_CurrentlySelectedUniform = 0;
	};
}