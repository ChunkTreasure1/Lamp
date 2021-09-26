#include "lppch.h"
#include "RenderGraphLoader.h"

#include "yaml-cpp/yaml.h"
#include "Lamp/Rendering/RenderGraph/RenderGraph.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"

namespace Lamp
{
	void RenderGraphLoader::Save(const Ref<Asset>& asset) const
	{
		Ref<RenderGraph> graph = std::dynamic_pointer_cast<RenderGraph>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "RenderGraph" << YAML::Value;
		{
			out << YAML::BeginMap;

			LP_SERIALIZE_PROPERTY(name, graph->GetSpecification().name, out);

			out << YAML::Key << "Nodes" << YAML::Value;
			out << YAML::BeginMap;
			int i = 0;
			for (auto& node : graph->GetSpecification().nodes)
			{
				std::string id = std::to_string(i);
				out << YAML::Key << "Node" + id << YAML::Value;
				out << YAML::BeginMap;
				{
					LP_SERIALIZE_PROPERTY(id, node->id, out);
					LP_SERIALIZE_PROPERTY(position, node->position, out);

					//Attributes
				}
				out << YAML::EndMap; //Node
			}
			out << YAML::EndMap; //Nodes

			out << YAML::EndMap;
		}
	}

	bool RenderGraphLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		return false;
	}
}