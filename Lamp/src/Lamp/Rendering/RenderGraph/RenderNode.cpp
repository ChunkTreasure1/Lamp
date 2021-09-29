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
			ImNodes::BeginInputAttribute(input->id, ImNodesPinShape_TriangleFilled);

			ImGui::Text(input->name.c_str());

			ImNodes::EndInputAttribute();
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

    void RenderNode::SerializeBaseAttribute(Ref<RenderAttribute> attr, const std::string& attrTypeS, YAML::Emitter& out, GraphUUID id)
    {
        out << YAML::Key << "attribute" + std::to_string(id) << YAML::Value;
        out << YAML::BeginMap;
        {
			//Base
			LP_SERIALIZE_PROPERTY(attrType, attrTypeS, out);
			LP_SERIALIZE_PROPERTY(id, attr->id, out);
			LP_SERIALIZE_PROPERTY(name, attr->name, out);
			LP_SERIALIZE_PROPERTY(type, (uint32_t)attr->type, out);
        }
        out << YAML::EndMap;
    }

    std::pair<Ref<RenderAttribute>, std::string> RenderNode::DeserializeBaseAttribute(const YAML::Node& node)
    {
        std::string nodeType = node["attrType"].as<std::string>();
        Ref<RenderAttribute> attr;
        if (nodeType == "input")
        {
            attr = CreateRef<RenderInputAttribute>();
        }
        else
        {
            attr = CreateRef<RenderOutputAttribute>();
        }

        LP_DESERIALIZE_PROPERTY(id, attr->id, node, 0);
        attr->name = node["name"].as<std::string>();
        attr->type = (RenderAttributeType)node["type"].as<uint32_t>();

        return std::make_pair(attr, nodeType);
    }
}