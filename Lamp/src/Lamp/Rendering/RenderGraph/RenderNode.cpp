#include "lppch.h"
#include "RenderNode.h"

#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "Lamp/Utility/SerializeMacros.h"

namespace Lamp
{
    void RenderNode::SerializeBaseAttribute(Ref<RenderAttribute> attr, const std::string& attrTypeS, YAML::Emitter& out, uint32_t id)
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