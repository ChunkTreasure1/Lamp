#include "lppch.h"
#include "RenderGraphLoader.h"

#include "Lamp/Rendering/RenderGraph/RenderGraph.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Utility/YAMLSerializationHelpers.h"

#include "Lamp/Rendering/RenderGraph/Nodes/RenderNodeTexture.h"
#include "Lamp/Rendering/RenderGraph/Nodes/RenderNodeFramebuffer.h"
#include "Lamp/Rendering/RenderGraph/Nodes/RenderNodeDynamicUniform.h"
#include "Lamp/Rendering/RenderGraph/Nodes/RenderNodePass.h"
#include "Lamp/Rendering/RenderGraph/Nodes/RenderNodeStart.h"
#include "Lamp/Rendering/RenderGraph/Nodes/RenderNodeEnd.h"
#include "Lamp/Rendering/RenderGraph/Nodes/RenderNodeCompute.h"
#include "ResourceCache.h"

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
			LP_SERIALIZE_PROPERTY(handle, graph->Handle, out);

			out << YAML::Key << "Nodes" << YAML::BeginSeq;
			for (const auto& node : graph->GetSpecification().nodes)
			{
				out << YAML::BeginMap;
				{
					out << YAML::Key << "Node" << YAML::Value << "";
					LP_SERIALIZE_PROPERTY(id, node->id, out);
					LP_SERIALIZE_PROPERTY(position, node->position, out);
					LP_SERIALIZE_PROPERTY(type, (uint32_t)node->GetNodeType(), out);

					node->Serialize(out);

				}
				out << YAML::EndMap; //Node
			}
			out << YAML::EndSeq; //Nodes

			out << YAML::Key << "Links" << YAML::BeginSeq;
			{
				for (const auto& link : graph->GetSpecification().links)
				{
					out << YAML::BeginMap;
					out << YAML::Key << "Link" << YAML::Value << "";
					LP_SERIALIZE_PROPERTY(id, link->id, out);
					LP_SERIALIZE_PROPERTY(from, (link->pOutput ? link->pOutput->id : -1), out);
					LP_SERIALIZE_PROPERTY(to, (link->pInput ? link->pInput->id : -1), out);

					out << YAML::EndMap;
				}
			}
			out << YAML::EndSeq; //Links

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream fout(asset->Path);
		fout << out.c_str();
		fout.close();
	}

	bool RenderGraphLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		std::ifstream stream(path);
		if (!stream.is_open())
		{
			return false;
		}

		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node root = YAML::Load(strStream.str());
		YAML::Node graphNode = root["RenderGraph"];

		asset = CreateRef<RenderGraph>();
		Ref<RenderGraph> graph = std::dynamic_pointer_cast<RenderGraph>(asset);
		auto& specification = graph->GetSpecification();

		specification.name = graphNode["name"].as<std::string>();
		LP_DESERIALIZE_PROPERTY(handle, graph->Handle, graphNode, (AssetHandle)0);

		//Nodes
		YAML::Node nodesNode = graphNode["Nodes"];
		uint32_t nodeCount = 0;

		for (auto entry : nodesNode)
		{
			RenderNodeType type = (RenderNodeType)entry["type"].as<uint32_t>();

			switch (type)
			{
				case RenderNodeType::Pass:
				{
					Ref<RenderNodePass> node = CreateRef<RenderNodePass>();
					node->Initialize();
					node->Deserialize(entry);

					specification.nodes.push_back(node);
					break;
				}

				case RenderNodeType::Texture:
				{
					Ref<RenderNodeTexture> node = CreateRef<RenderNodeTexture>();
					node->Initialize();
					node->Deserialize(entry);

					specification.nodes.push_back(node);
					break;
				}

				case RenderNodeType::Framebuffer:
				{
					Ref<RenderNodeFramebuffer> node = CreateRef<RenderNodeFramebuffer>();
					node->Initialize();
					node->Deserialize(entry);

					specification.nodes.push_back(node);
					break;
				}

				case RenderNodeType::DynamicUniform:
				{
					Ref<RenderNodeDynamicUniform> node = CreateRef<RenderNodeDynamicUniform>();
					node->Initialize();
					node->Deserialize(entry);

					specification.nodes.push_back(node);
					break;
				}

				case RenderNodeType::Start:
				{
					Ref<RenderNodeStart> node = CreateRef<RenderNodeStart>();
					node->Initialize();
					node->Deserialize(entry);

					specification.nodes.push_back(node);
					specification.startNode = node;
					break;
				}

				case RenderNodeType::End:
				{
					Ref<RenderNodeEnd> node = CreateRef<RenderNodeEnd>();
					node->Initialize();
					node->Deserialize(entry);

					specification.nodes.push_back(node);
					specification.endNode = node;
					break;
				}

				case RenderNodeType::Compute:
				{
					Ref<RenderNodeCompute> node = CreateRef<RenderNodeCompute>();
					node->Initialize();
					node->Deserialize(entry);

					specification.nodes.push_back(node);

					break;
				}

				default:
					LP_CORE_ASSERT(false, "Node type not found!");
					break;
			}


			Ref<RenderNode> node = specification.nodes[specification.nodes.size() - 1];
			LP_DESERIALIZE_PROPERTY(id, node->id, entry, 0);
			LP_DESERIALIZE_PROPERTY(position, node->position, entry, glm::vec2(0.f));
		}

		//Links
		YAML::Node linksNode = graphNode["Links"];
		
		for (auto entry : linksNode)
		{
			Ref<RenderLink> link = CreateRef<RenderLink>();
			LP_DESERIALIZE_PROPERTY(id, link->id, entry, 0);

			uint32_t from = 0;
			uint32_t to = 0;
			LP_DESERIALIZE_PROPERTY(from, from, entry, 0);
			LP_DESERIALIZE_PROPERTY(to, to, entry, 0);

			for (const auto& node : specification.nodes)
			{
				for (const auto& input : node->inputs)
				{
					if (input->id == to)
					{
						link->pInput = input.get();
						input->pNode->links.push_back(link);

						break;
					}
				}

				for (const auto& output : node->outputs)
				{
					if (output->id == from)
					{
						link->pOutput = output.get();
						output->pNode->links.push_back(link);
						break;
					}
				}
			}

			specification.links.push_back(link);
		}

		graph->Path = path;

		return true;
	}
}