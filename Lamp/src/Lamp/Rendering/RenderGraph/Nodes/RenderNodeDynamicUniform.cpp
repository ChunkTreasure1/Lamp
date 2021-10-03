#include "lppch.h"
#include "RenderNodeDynamicUniform.h"

#include "RenderNodePass.h"
#include "DynamicUniformRegistry.h"

#include "Lamp/Utility/SerializeMacros.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imnodes.h>

namespace Lamp
{
	void RenderNodeDynamicUniform::Initialize()
	{
		auto& uniforms = DynamicUniformRegistry::s_Uniforms();
		for (auto& uniform : uniforms)
		{
			auto& [name, type, data] = uniform;

			m_Uniforms.push_back(name.c_str());
		}

		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->pNode = this;
		output->name = "Output";
		output->type = RenderAttributeType::DynamicUniform;

		outputs.push_back(output);
	}

	void RenderNodeDynamicUniform::Start()
	{
		for (const auto& link : links)
		{
			if (RenderNodePass* passNode = dynamic_cast<RenderNodePass*>(link->pInput->pNode))
			{
				GraphUUID id = std::any_cast<GraphUUID>(link->pInput->data);
				passNode->renderPass->GetSpecification().dynamicUniforms[id].first.data = pData;
				passNode->renderPass->GetSpecification().dynamicUniforms[id].first.type = uniformType;
			}
		}
	}

	void RenderNodeDynamicUniform::DrawNode()
	{
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(153, 64, 173, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(159, 94, 173, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(159, 94, 173, 255));

		ImNodes::BeginNode(id);

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(id);
		if (pos.x != position.x || pos.y != position.y)
		{
			position = { pos.x, pos.y };
		}

		ImNodes::BeginNodeTitleBar();
		ImGui::Text("Dynamic Uniform node");
		ImNodes::EndNodeTitleBar();

		ImGui::PushItemWidth(150.f);

		if (ImGui::Combo("Uniform", &m_CurrentlySelectedUniform, m_Uniforms.data(), (int)m_Uniforms.size()))
		{
			auto& [name, uType, data] = DynamicUniformRegistry::s_Uniforms()[m_CurrentlySelectedUniform];
			uniformType = uType;
			pData = data;
			dataName = name;
		}

		DrawAttributes();

		ImGui::PopItemWidth();

		ImNodes::EndNode();

		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeDynamicUniform::Serialize(YAML::Emitter& out)
	{
		LP_SERIALIZE_PROPERTY(uniformType, (uint32_t)uniformType, out);
		LP_SERIALIZE_PROPERTY(dataName, dataName, out);

		SerializeAttributes(out);
	}

	void RenderNodeDynamicUniform::Deserialize(YAML::Node& node)
	{
		uniformType = (UniformType)node["uniformType"].as<uint32_t>();
		dataName = node["dataName"].as<std::string>();

		int i = 0;
		for (const auto& [uName, uType, uData] : DynamicUniformRegistry::s_Uniforms())
		{
			if (uName == dataName)
			{
				m_CurrentlySelectedUniform = i;
				pData = uData;
				break;
			}
			i++;
		}

		//attributes
		outputs.clear();
		inputs.clear();

		YAML::Node attributesNode = node["attributes"];
		DeserializeAttributes(attributesNode);
	}
}