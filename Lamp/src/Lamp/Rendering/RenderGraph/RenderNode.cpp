#include "lppch.h"
#include "RenderNode.h"

#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "Lamp/Utility/SerializeMacros.h"

#include <imnodes.h>

namespace Lamp
{
	void RenderNode::DrawAttributes()
	{
		ImVec2 cursorPos = ImGui::GetCursorPos();
		cursorPos.x += 150.f;

		if (!inputs.empty())
		{
			ImGui::TextColored(ImVec4(0.38f, 0.42f, 1.f, 1.f), "Inputs");
		}

		for (auto& input : inputs)
		{
			unsigned int pinColor = ImNodes::GetStyle().Colors[ImNodesCol_Pin];
			unsigned int pinHoverColor = ImNodes::GetStyle().Colors[ImNodesCol_PinHovered];

			switch (input->type)
			{
				case RenderAttributeType::Pass:
					break;

				case RenderAttributeType::DynamicUniform:
					pinColor = IM_COL32(153, 64, 173, 255);
					pinHoverColor = IM_COL32(159, 94, 173, 255);
					break;

				case RenderAttributeType::Framebuffer:
					pinColor = IM_COL32(150, 28, 17, 255);
					pinHoverColor = IM_COL32(179, 53, 41, 255);
					break;

				case RenderAttributeType::Texture:
					pinColor = IM_COL32(62, 189, 100, 255);
					pinHoverColor = IM_COL32(100, 181, 124, 255);
					break;

				default:
					break;
			}

			ImNodes::PushColorStyle(ImNodesCol_Pin, pinColor);
			ImNodes::PushColorStyle(ImNodesCol_PinHovered, pinHoverColor);
			ImNodes::BeginInputAttribute(input->id, ImNodesPinShape_TriangleFilled);

			ImGui::Text(input->name.c_str());

			ImNodes::EndInputAttribute();
			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
		}

		ImGui::SetCursorPos(cursorPos);

		if (!outputs.empty())
		{
			ImGui::TextColored(ImVec4(0.101f, 1.f, 0.313f, 1.f), "Outputs");
		}

		for (auto& output : outputs)
		{
			cursorPos.y += 20.f;
			ImGui::SetCursorPos(cursorPos);

			ImNodes::BeginOutputAttribute(output->id);
			ImGui::Text(output->name.c_str());
			ImNodes::EndOutputAttribute();
		}
	}

	void RenderNode::SerializeBaseAttribute(Ref<RenderAttribute> attr, const std::string& attrTypeS, YAML::Emitter& out)
	{
		out << YAML::BeginMap;
		{
			//Base
			out << YAML::Key << "attribute" << YAML::Value << attr->name;
			LP_SERIALIZE_PROPERTY(attrType, attrTypeS, out);
			LP_SERIALIZE_PROPERTY(id, attr->id, out);
			LP_SERIALIZE_PROPERTY(type, (uint32_t)attr->type, out);

			if (attrTypeS == "input")
			{
				Ref<RenderInputAttribute> inAttr = std::dynamic_pointer_cast<RenderInputAttribute>(attr);
				if (inAttr->data.type() == typeid(GraphUUID))
				{
					LP_SERIALIZE_PROPERTY(data, std::any_cast<GraphUUID>(inAttr->data), out);
				}
				else
				{
					LP_SERIALIZE_PROPERTY(data, 0, out);
				}
			}
		}
		out << YAML::EndMap;
	}

	void RenderNode::SerializeAttributes(YAML::Emitter& out)
	{
		out << YAML::Key << "attributes" << YAML::BeginSeq;
		for (auto& input : inputs)
		{
			SerializeBaseAttribute(input, "input", out);
		}

		for (auto& output : outputs)
		{
			SerializeBaseAttribute(output, "output", out);
		}
		out << YAML::EndSeq;
	}

	void RenderNode::DeserializeAttributes(YAML::Node& node)
	{
		for (const auto entry : node)
		{
			const auto& [attr, attrType] = DeserializeBaseAttribute(entry);
			attr->pNode = this;
			if (attrType == "input")
			{
				inputs.push_back(std::dynamic_pointer_cast<RenderInputAttribute>(attr));
			}
			else
			{
				outputs.push_back(std::dynamic_pointer_cast<RenderOutputAttribute>(attr));
			}
		}
	}

	std::pair<Ref<RenderAttribute>, std::string> RenderNode::DeserializeBaseAttribute(const YAML::Node& node)
	{
		std::string nodeType = node["attrType"].as<std::string>();
		Ref<RenderAttribute> attr;
		if (nodeType == "input")
		{
			attr = CreateRef<RenderInputAttribute>();
			auto& inAttr = std::dynamic_pointer_cast<RenderInputAttribute>(attr);
			GraphUUID id;
			LP_DESERIALIZE_PROPERTY(data, id, node, (GraphUUID)0);

			inAttr->data = id;
		}
		else
		{
			attr = CreateRef<RenderOutputAttribute>();
		}

		LP_DESERIALIZE_PROPERTY(id, attr->id, node, 0);
		attr->name = node["attribute"].as<std::string>();
		attr->type = (RenderAttributeType)node["type"].as<uint32_t>();

		return std::make_pair(attr, nodeType);
	}
}