#include "lppch.h"
#include "RenderNodeEnd.h"

#include "RenderNodePass.h"

#include <imnodes.h>

namespace Lamp
{
	void RenderNodeEnd::Initialize()
	{
		Ref<RenderInputAttribute> endAttr = CreateRef<RenderInputAttribute>();
		endAttr->pNode = this;
		endAttr->name = "End";
		endAttr->id = ++currId;
		endAttr->type = RenderAttributeType::Pass;

		inputs.push_back(endAttr);
	}

	void RenderNodeEnd::Start()
	{}

	void RenderNodeEnd::DrawNode()
	{
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(36, 166, 151, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(107, 194, 184, 255));
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(107, 194, 184, 255));

		ImNodes::BeginNode(id);

		ImNodes::BeginNodeTitleBar();
		ImGui::Text("End");
		ImNodes::EndNodeTitleBar();

		for (auto& input : inputs)
		{
			ImNodes::BeginInputAttribute(input->id);
			ImGui::Text(input->name.c_str());
			ImNodes::EndInputAttribute();
		}

		ImNodes::EndNode();

		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
	}

	void RenderNodeEnd::Activate(std::any value)
	{
		LP_PROFILE_FUNCTION();
		framebuffer = std::any_cast<Ref<Framebuffer>>(value);
	}

	void RenderNodeEnd::Serialize(YAML::Emitter& out)
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

	void RenderNodeEnd::Deserialize(YAML::Node& node)
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

			currId = attr->id;
			attributeCount++;
		}
	}
}