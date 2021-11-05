#include "lppch.h"
#include "RenderNode.h"

#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "RenderGraphUtils.h"

#include <imnodes.h>

namespace Lamp
{
	void RenderNode::DrawAttributes(const std::vector<Ref<RenderInputAttribute>>& aInputs, const std::vector<Ref<RenderOutputAttribute>>& aOutputs)
	{
		ImVec2 cursorPos = ImGui::GetCursorPos();

		if (GetNodeType() != RenderNodeType::Start)
		{
			cursorPos.x += ImNodes::GetNodeDimensions(id).x - 95.f;
		}

		if (!aInputs.empty())
		{
			ImGui::TextColored(ImVec4(0.38f, 0.42f, 1.f, 1.f), "Inputs");
		}

		for (auto& input : aInputs)
		{
			unsigned int pinColor = Utils::GetTypeColor(input->type);
			unsigned int pinHoverColor = Utils::GetTypeHoverColor(input->type);

			ImNodes::PushColorStyle(ImNodesCol_Pin, pinColor);
			ImNodes::PushColorStyle(ImNodesCol_PinHovered, pinHoverColor);

			ImNodesPinShape pinShape = IsAttributeLinked(input) ? ImNodesPinShape_TriangleFilled : ImNodesPinShape_Triangle;

			ImNodes::BeginInputAttribute(input->id, pinShape);

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
			cursorPos.y += 22.f;
			ImGui::SetCursorPos(cursorPos);

			unsigned int pinColor = Utils::GetTypeColor(output->type);
			unsigned int pinHoverColor = Utils::GetTypeHoverColor(output->type);

			ImNodes::PushColorStyle(ImNodesCol_Pin, pinColor);
			ImNodes::PushColorStyle(ImNodesCol_PinHovered, pinHoverColor);

			ImNodesPinShape pinShape = IsAttributeLinked(output) ? ImNodesPinShape_CircleFilled : ImNodesPinShape_Circle;

			ImNodes::BeginOutputAttribute(output->id, pinShape);
			ImGui::Text(output->name.c_str());
			ImNodes::EndOutputAttribute();

			ImNodes::PopColorStyle();
			ImNodes::PopColorStyle();
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

	bool RenderNode::IsAttributeLinked(Ref<RenderAttribute> attr)
	{
		for (const auto& link : links)
		{
			if (link->pInput->id == attr->id)
			{
				return true;
			}

			if (link->pOutput->id == attr->id)
			{
				return true;
			}
		}

		return false;
	}

	Ref<RenderAttribute> RenderNode::FindAttributeByID(GraphUUID id)
	{
		for (auto& input : inputs)
		{
			if (input->id == id)
			{
				return input;
			}
		}

		for (auto& output : outputs)
		{
			if (output->id == id)
			{
				return output;
			}
		}

		return nullptr;
	}
}