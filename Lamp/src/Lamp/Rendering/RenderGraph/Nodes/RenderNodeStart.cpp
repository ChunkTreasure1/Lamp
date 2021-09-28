#include "lppch.h"
#include "RenderNodeStart.h"

#include "RenderNodePass.h"
#include <imnodes.h>

namespace Lamp
{
	void RenderNodeStart::Initialize()
	{
		Ref<RenderOutputAttribute> output = CreateRef<RenderOutputAttribute>();
		output->pNode = this;
		output->name = "Start";
		output->type = RenderAttributeType::Pass;

		outputs.push_back(output);
	}

	void RenderNodeStart::Start()
	{}

	void RenderNodeStart::DrawNode()
	{
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(36, 166, 151, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(107, 194, 184, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(107, 194, 184, 255));

		ImNodes::BeginNode(id);

		ImVec2 pos = ImNodes::GetNodeEditorSpacePos(id);
		if (pos.x != position.x || pos.y != position.y)
		{
			position = { pos.x, pos.y };
		}

		ImNodes::BeginNodeTitleBar();
		ImGui::Text("Start");
		ImNodes::EndNodeTitleBar();

		for (auto& output : outputs)
		{
			ImNodes::BeginOutputAttribute(output->id);
			ImGui::Text(output->name.c_str());
			ImNodes::EndOutputAttribute();
		}

		ImNodes::EndNode();

		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeStart::Activate(std::any value)
	{
		Renderer3D::Begin(std::any_cast<Ref<CameraBase>>(value));
		for (const auto& link : links)
		{
			if (link->pInput->pNode->GetNodeType() == RenderNodeType::Pass)
			{
				link->pInput->pNode->Activate(value);
			}
		}
		Renderer3D::End();
	}

	void RenderNodeStart::Serialize(YAML::Emitter& out)
	{
		uint32_t attrId = 0;
		for (auto& input : inputs)
		{
			SerializeBaseAttribute(input, "input", out, attrId);
			attrId++;
		}

		for (auto& output : outputs)
		{
			SerializeBaseAttribute(output, "output", out, attrId);
			attrId++;
		}
	}

	void RenderNodeStart::Deserialize(YAML::Node& node)
	{
		//attributes
		outputs.clear();
		inputs.clear();
		uint32_t attributeCount = 0;

		while (YAML::Node attribute = node["attribute" + std::to_string(attributeCount)])
		{
			const auto& [attr, attrType] = DeserializeBaseAttribute(attribute);
			attr->pNode = this;
			if (attrType == "input")
			{
				inputs.push_back(std::dynamic_pointer_cast<RenderInputAttribute>(attr));
			}
			else
			{
				outputs.push_back(std::dynamic_pointer_cast<RenderOutputAttribute>(attr));
			}

			attributeCount++;
		}
	}
}