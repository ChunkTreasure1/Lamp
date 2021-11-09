#pragma once

#include "Lamp/Rendering/RenderGraph/RenderNode.h"

namespace Lamp
{
	class RenderNodeDynamicUniform : public RenderNode
	{
	public:
		RenderNodeDynamicUniform()
			: uniformType(UniformType::Int)
		{}

		void Initialize() override;
		void Start() override;
		void DrawNode() override;
		void Activate(std::any value) override {}
		RenderNodeType GetNodeType() override { return RenderNodeType::DynamicUniform; }
		void Serialize(YAML::Emitter& node) override;
		void Deserialize(YAML::Node& node) override;

		void* pData = nullptr;
		UniformType uniformType;
		std::string dataName;

	private:
		std::vector<const char*> m_Uniforms;
		int m_CurrentlySelectedUniform = 0;
	};
}