#include "lppch.h"
#include "LevelLoader.h"

#include "Lamp/AssetSystem/AssetManager.h"
#include "Lamp/Core/Application.h"

#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "Lamp/Utility/SerializeMacros.h"

#include "Lamp/Level/Level.h"
#include "Lamp/Rendering/Textures/Texture2D.h"

#include "Lamp/Objects/Brushes/Brush.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/GraphKey/GraphKeyGraph.h"
#include "Lamp/GraphKey/NodeRegistry.h"

#include <yaml-cpp/yaml.h>

namespace Lamp
{
	void LevelLoader::Save(const Ref<Asset>& asset) const
	{
		Ref<Level> level = std::dynamic_pointer_cast<Level>(asset);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "level" << YAML::Value;
		{
			out << YAML::BeginMap;

			LP_SERIALIZE_PROPERTY(name, level->GetName(), out);
			LP_SERIALIZE_PROPERTY(handle, level->Handle, out);

			out << YAML::Key << "layers" << YAML::BeginSeq;
			for (const auto& layer : level->GetLayers())
			{
				out << YAML::BeginMap;
				LP_SERIALIZE_PROPERTY(layer, layer.Name, out);
				LP_SERIALIZE_PROPERTY(id, layer.ID, out);
				LP_SERIALIZE_PROPERTY(active, layer.Active, out);
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "Environment" << YAML::Value;
			out << YAML::BeginMap;
			{
				LP_SERIALIZE_PROPERTY(cameraPosition, level->GetEnvironment().GetCameraPosition(), out);
				LP_SERIALIZE_PROPERTY(cameraRotation, level->GetEnvironment().GetCameraRotation(), out);
				LP_SERIALIZE_PROPERTY(cameraFOV, level->GetEnvironment().GetCameraFOV(), out);

				out << YAML::Key << "Skybox" << YAML::Value;
				out << YAML::BeginMap;
				{
					LP_SERIALIZE_PROPERTY(environmentLod, level->GetEnvironment().GetSkybox().environmentLod, out);
					LP_SERIALIZE_PROPERTY(environmentMultiplier, level->GetEnvironment().GetSkybox().environmentMultiplier, out);
					LP_SERIALIZE_PROPERTY(skybox, level->GetEnvironment().GetSkybox().skybox->Path.string(), out);
				}
				out << YAML::EndMap;
			
				out << YAML::Key << "Terrain" << YAML::Value;
				out << YAML::BeginMap;
				{
					LP_SERIALIZE_PROPERTY(terrainScale, level->GetEnvironment().GetTerrain().terrainScale, out);
					LP_SERIALIZE_PROPERTY(terrainShift, level->GetEnvironment().GetTerrain().terrainShift, out);
					LP_SERIALIZE_PROPERTY(terrain, level->GetEnvironment().GetTerrain().terrain->GetHeightMap()->Path.string(), out);
				}
				out << YAML::EndMap;
			}
			out << YAML::EndMap;

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		if (!std::filesystem::exists(asset->Path.parent_path()))
		{
			std::filesystem::create_directory(asset->Path.parent_path());
		}

		std::ofstream file;
		file.open(asset->Path);
		file << out.c_str();
		file.close();

		for (const auto& layer : level->GetLayers())
		{
			YAML::Emitter layerOut;

			std::vector<Object*> entities;
			std::vector<Object*> brushes;

			for (const auto& obj : layer.Objects)
			{
				if (typeid(*obj) == typeid(Brush))
				{
					brushes.push_back(obj);
				}
				else
				{
					entities.push_back(obj);
				}
			}

			layerOut << YAML::BeginMap;
			layerOut << YAML::Key << "layer" << YAML::Value;
			{
				layerOut << YAML::BeginMap;
				LP_SERIALIZE_PROPERTY(name, layer.Name, layerOut);

				LP_SERIALIZE_PROPERTY(id, layer.ID, layerOut);
				LP_SERIALIZE_PROPERTY(active, layer.Active, layerOut);

				layerOut << YAML::Key << "entities" << YAML::BeginSeq;
				for (const auto& ent : entities)
				{
					SerializeEntity(layerOut, dynamic_cast<Entity*>(ent));
				}
				layerOut << YAML::EndSeq;

				layerOut << YAML::Key << "brushes" << YAML::BeginSeq;
				for (const auto& brush : brushes)
				{
					SerializeBrush(layerOut, dynamic_cast<Brush*>(brush));
				}
				layerOut << YAML::EndSeq;

				layerOut << YAML::EndMap;
			}
			layerOut << YAML::EndMap;

			std::ofstream file;
			auto path = asset->Path.parent_path() / std::filesystem::path("layers");

			if (!std::filesystem::exists(path))
			{
				std::filesystem::create_directory(path);
			}

			path /= std::filesystem::path(layer.Name + ".lyr");

			file.open(path);
			file << layerOut.c_str();
			file.close();
		}
	}

	bool LevelLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		LevelLoadStartedEvent startEvent(path);
		Application::Get().OnEvent(startEvent);

		asset = CreateRef<Level>();
		Ref<Level> level = std::dynamic_pointer_cast<Level>(asset);
		LevelManager::Get()->SetActive(level);

		if (!std::filesystem::exists(path))
		{
			asset->SetFlag(AssetFlag::Missing);
			return false;
		}

		std::ifstream levelFile(path);
		if (!levelFile.is_open())
		{
			asset->SetFlag(AssetFlag::Invalid);
			return false;
		}

		std::stringstream strStream;
		strStream << levelFile.rdbuf();

		YAML::Node root = YAML::Load(strStream.str());
		YAML::Node levelNode = root["level"];

		level->m_name = levelNode["name"].as<std::string>();
		LP_DESERIALIZE_PROPERTY(handle, level->Handle, levelNode, (AssetHandle)0);

		//Environment
		{
			YAML::Node envNode = levelNode["Environment"];

			LP_DESERIALIZE_PROPERTY(cameraPosition, const_cast<glm::vec3&>(level->m_environment.GetCameraPosition()), envNode, glm::vec3(0.f));
			LP_DESERIALIZE_PROPERTY(cameraRotation, const_cast<glm::quat&>(level->m_environment.GetCameraRotation()), envNode, glm::quat());
			LP_DESERIALIZE_PROPERTY(cameraFOV, const_cast<float&>(level->m_environment.GetCameraFOV()), envNode, 60.f);
		
			//Terrain
			{
				YAML::Node terrainNode = envNode["Terrain"];

				auto& terrain = const_cast<TerrainData&>(level->m_environment.GetTerrain());
				LP_DESERIALIZE_PROPERTY(terrainScale, terrain.terrainScale, terrainNode, 64.f);
				LP_DESERIALIZE_PROPERTY(terrainShift, terrain.terrainShift, terrainNode, 16.f);
				
				std::string terrPath = terrainNode["terrain"] ? terrainNode["terrain"].as<std::string>() : "";
				terrain.terrain = Terrain::Create(terrPath);
			}

			//Skybox
			{
				YAML::Node skyboxNode = envNode["Skybox"];

				auto& skybox = const_cast<SkyboxData&>(level->m_environment.GetSkybox());
				LP_DESERIALIZE_PROPERTY(environmentLod, skybox.environmentLod, skyboxNode, 1.f);
				LP_DESERIALIZE_PROPERTY(environmentMultiplier, skybox.environmentMultiplier, skyboxNode, 1.f);
			
				std::string skyboxPath = skyboxNode["skybox"] ? skyboxNode.as<std::string>() : "";
				skybox.skybox = Skybox::Create(skyboxPath);
			}
		}

		//Layers
		YAML::Node layersNode = levelNode["layers"];
		std::vector<ObjectLayer> layers;

		for (auto entry : layersNode)
		{
			std::string name = entry["layer"].as<std::string>();
			uint32_t id = entry["id"].as<uint32_t>();
			bool active = entry["active"].as<bool>();

			layers.emplace_back(name, id, active);
		}

		std::map<uint32_t, Brush*> brushes;
		std::map<uint32_t, Entity*> entities;

		//Layer files
		for (auto& layer : layers)
		{
			auto layerPath = path.parent_path() / std::filesystem::path("layers/" + layer.Name + ".lyr");
			std::ifstream layerFile(layerPath);
			if (!levelFile.is_open())
			{
				continue;
			}

			std::stringstream layerStream;
			layerStream << layerFile.rdbuf();

			YAML::Node layerRoot = YAML::Load(layerStream.str());
			YAML::Node layerNode = layerRoot["layer"];

			//Brushes
			YAML::Node brushesNode = layerNode["brushes"];
			for (const auto entry : brushesNode)
			{
				DeserializeBrush(entry, brushes);
			}

			//Entities
			YAML::Node entitiesNode = layerNode["entities"];
			for (auto entry : entitiesNode)
			{
				DeserializeEntity(entry, entities);
			}
		}

		level->m_brushes = brushes;
		level->m_entities = entities;
		level->m_layers = layers;

		if (level->m_layers.empty())
		{
			level->AddLayer(ObjectLayer("Main", 0, true));
		}

		for (const auto& entity : entities)
		{
			level->AddToLayer(entity.second);
		}

		for (const auto& brush : brushes)
		{
			level->AddToLayer(brush.second);
		}

		level->Path = path;

		LevelLoadFinishedEvent endEvent(path);
		Application::Get().OnEvent(endEvent);

		return true;
	}

	void LevelLoader::SerializeEntity(YAML::Emitter& out, const Entity* entity) const	
	{
		if (entity == nullptr)
		{
			return;
		}

		if (!entity->GetSaveable())
		{
			return;
		}

		out << YAML::BeginMap;
		LP_SERIALIZE_PROPERTY(entity, entity->GetName(), out);
		LP_SERIALIZE_PROPERTY(position, entity->GetPosition(), out);
		LP_SERIALIZE_PROPERTY(rotation, entity->GetRotation(), out);
		LP_SERIALIZE_PROPERTY(scale, entity->GetScale(), out);
		LP_SERIALIZE_PROPERTY(layerId, entity->GetLayerID(), out);

		out << YAML::Key << "components" << YAML::BeginSeq;
		for (const auto& component : entity->GetComponents())
		{
			std::string str = component->GetName();
			str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());

			out << YAML::BeginMap;
			LP_SERIALIZE_PROPERTY(name, str, out);

			out << YAML::Key << "params" << YAML::BeginSeq;
			for (const auto& prop : component->GetComponentProperties().GetProperties())
			{
				out << YAML::BeginMap;
				LP_SERIALIZE_PROPERTY(param, prop.name, out);
				LP_SERIALIZE_PROPERTY(type, ComponentProperties::GetStringFromType(prop.propertyType), out);

				switch (prop.propertyType)
				{
					case PropertyType::String:
						LP_SERIALIZE_PROPERTY(value, *static_cast<std::string*>(prop.value), out);
						break;

					case PropertyType::Bool:
						LP_SERIALIZE_PROPERTY(value, *static_cast<bool*>(prop.value), out);
						break;

					case PropertyType::Int:
						LP_SERIALIZE_PROPERTY(value, *static_cast<int*>(prop.value), out);
						break;

					case PropertyType::Float:
						LP_SERIALIZE_PROPERTY(value, *static_cast<float*>(prop.value), out);
						break;

					case PropertyType::Float2:
						LP_SERIALIZE_PROPERTY(value, *static_cast<glm::vec2*>(prop.value), out);
						break;

					case PropertyType::Float3:
						LP_SERIALIZE_PROPERTY(value, *static_cast<glm::vec3*>(prop.value), out);
						break;

					case PropertyType::Float4:
						LP_SERIALIZE_PROPERTY(value, *static_cast<glm::vec4*>(prop.value), out);
						break;

					case PropertyType::Path:
						LP_SERIALIZE_PROPERTY(value, *static_cast<std::string*>(prop.value), out);
						break;

					case PropertyType::Color3:
						LP_SERIALIZE_PROPERTY(value, *static_cast<glm::vec3*>(prop.value), out);
						break;

					case PropertyType::Color4:
						LP_SERIALIZE_PROPERTY(value, *static_cast<glm::vec4*>(prop.value), out);
						break;

					default:
						break;
				}

				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		//Save entities graph
		if (entity->GetGraphKeyGraph())
		{
			auto graph = entity->GetGraphKeyGraph();

			out << YAML::Key << "graphkey" << YAML::BeginMap;
			LP_SERIALIZE_PROPERTY(name, graph->GetSpecification().name, out);

			//Nodes
			out << YAML::Key << "nodes" << YAML::BeginSeq;
			for (const auto& node : graph->GetSpecification().nodes)
			{
				out << YAML::BeginMap;
				LP_SERIALIZE_PROPERTY(name, node->name, out);
				LP_SERIALIZE_PROPERTY(id, node->id, out);
				LP_SERIALIZE_PROPERTY(position, node->position, out);

				//Attributes
				out << YAML::Key << "attributes" << YAML::BeginSeq;
				for (const auto& attr : node->inputAttributes)
				{
					SerializeAttribute(attr, "input", out);
				}
				
				for (const auto& attr : node->outputAttributes)
				{
					SerializeAttribute(attr, "output", out);
				}
				out << YAML::EndSeq;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			//links
				out << YAML::Key << "links" << YAML::BeginSeq;
			for (const auto& link : graph->GetSpecification().links)
			{
				out << YAML::BeginMap;

				LP_SERIALIZE_PROPERTY(id, link->id, out);
				LP_SERIALIZE_PROPERTY(from, link->pOutput->id, out);
				LP_SERIALIZE_PROPERTY(to, link->pInput->id, out);

				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	void LevelLoader::SerializeBrush(YAML::Emitter& out, const Brush* pBrush) const
	{
		out << YAML::BeginMap;
		out << YAML::Key << "brush" << YAML::Value << pBrush->GetName();
		out << YAML::Key << "meshHandle" << YAML::Value << pBrush->GetModel()->Handle;
		out << YAML::Key << "position" << YAML::Value << pBrush->GetPosition();
		out << YAML::Key << "rotation" << YAML::Value << pBrush->GetRotation();
		out << YAML::Key << "scale" << YAML::Value << pBrush->GetScale();
		out << YAML::Key << "layerId" << YAML::Value << pBrush->GetLayerID();
		out << YAML::EndMap;
	}

	void LevelLoader::DeserializeEntity(const YAML::Node& entry, std::map<uint32_t, Entity*>& entities) const
	{
		uint32_t layerId = entry["layerId"].as<uint32_t>();
		Entity* entity = Entity::Create(true, layerId);

		std::string name = entry["entity"].as<std::string>();
		entity->SetName(name);

		glm::vec3 pos;
		LP_DESERIALIZE_PROPERTY(position, pos, entry, glm::vec3(0.f));

		glm::vec3 rot;
		LP_DESERIALIZE_PROPERTY(rotation, rot, entry, glm::vec3(0.f));

		glm::vec3 scale;
		LP_DESERIALIZE_PROPERTY(scale, scale, entry, glm::vec3(0.f));

		entity->SetPosition(pos);
		entity->SetRotation(rot);
		entity->SetScale(scale);

		YAML::Node componentsNode = entry["components"];
		for (auto compEntry : componentsNode)
		{
			std::string name = compEntry["name"].as<std::string>();
			Ref<EntityComponent> pComp = ComponentRegistry::Create(name);

			pComp->m_pEntity = entity;
			pComp->Initialize();

			YAML::Node paramsNode = compEntry["params"];
			for (auto paramEntry : paramsNode)
			{
				std::string paramName = paramEntry["param"].as<std::string>();
				PropertyType type = ComponentProperties::GetTypeFromString(paramEntry["type"].as<std::string>().c_str());

				switch (type)
				{
					case Lamp::PropertyType::String:
					{
						std::string* data = GetPropertyData<std::string>(paramName, pComp->GetComponentProperties().GetProperties());
						*data = paramEntry["value"].as<std::string>();
						break;
					}

					case Lamp::PropertyType::Path:
					{
						std::string* data = GetPropertyData<std::string>(paramName, pComp->GetComponentProperties().GetProperties());
						*data = paramEntry["value"].as<std::string>();
						break;
					}

					case Lamp::PropertyType::Bool:
					{
						bool* data = GetPropertyData<bool>(paramName, pComp->GetComponentProperties().GetProperties());
						*data = paramEntry["value"].as<bool>();
						break;
					}

					case Lamp::PropertyType::Int:
					{
						int* data = GetPropertyData<int>(paramName, pComp->GetComponentProperties().GetProperties());
						*data = paramEntry["value"].as<int>();
						break;
					}

					case Lamp::PropertyType::Float:
					{
						float* data = GetPropertyData<float>(paramName, pComp->GetComponentProperties().GetProperties());
						*data = paramEntry["value"].as<float>();
						break;
					}

					case Lamp::PropertyType::Float2:
					{
						glm::vec2* data = GetPropertyData<glm::vec2>(paramName, pComp->GetComponentProperties().GetProperties());
						*data = paramEntry["value"].as<glm::vec2>();
						break;
					}

					case Lamp::PropertyType::Float3:
					{
						glm::vec3* data = GetPropertyData<glm::vec3>(paramName, pComp->GetComponentProperties().GetProperties());
						*data = paramEntry["value"].as<glm::vec3>();
						break;
					}

					case Lamp::PropertyType::Float4:
					{
						glm::vec4* data = GetPropertyData<glm::vec4>(paramName, pComp->GetComponentProperties().GetProperties());
						*data = paramEntry["value"].as<glm::vec4>();
						break;
					}

					case Lamp::PropertyType::Color3:
					{
						glm::vec3* data = GetPropertyData<glm::vec3>(paramName, pComp->GetComponentProperties().GetProperties());
						*data = paramEntry["value"].as<glm::vec3>();
						break;
					}

					case Lamp::PropertyType::Color4:
					{
						glm::vec4* data = GetPropertyData<glm::vec4>(paramName, pComp->GetComponentProperties().GetProperties());
						*data = paramEntry["value"].as<glm::vec4>();
						break;
					}
					default:
						break;
				}

				ObjectPropertyChangedEvent e;
				pComp->OnEvent(e);
			}

			entity->AddComponent(pComp);
		}

		if (YAML::Node graphNode = entry["graphkey"])
		{
			uint32_t currentId = 0;
			GraphKeyGraphSpecification graphSpec{};
			graphSpec.name = graphNode["name"].as<std::string>();

			YAML::Node nodesNode = graphNode["nodes"];
			for (const auto& node : nodesNode)
			{
				Ref<Node> n = NodeRegistry::Create(node["name"].as<std::string>());
				LP_DESERIALIZE_PROPERTY(id, n->id, node, 0);
				if (n->id >= currentId)
				{
					currentId = n->id + 1;
				}

				LP_DESERIALIZE_PROPERTY(position, n->position, node, glm::vec2(0.f));

				YAML::Node attrNode = node["attributes"];
				for (const auto& attr : attrNode)
				{
					DeserializeAttribute(attr, n, currentId);
				}

				graphSpec.nodes.push_back(n);
			}

			YAML::Node linksNode = graphNode["links"];
			for (const auto& linkNode : linksNode)
			{
				Ref<Link> link = CreateRef<Link>();
				uint32_t from;
				uint32_t to;

				LP_DESERIALIZE_PROPERTY(id, link->id, linkNode, 0);
				LP_DESERIALIZE_PROPERTY(from, from, linkNode, 0);
				LP_DESERIALIZE_PROPERTY(to, to, linkNode, 0);

				if (link->id >= currentId)
				{
					currentId = link->id + 1;
				}

				for (const auto& node : graphSpec.nodes)
				{
					for (auto& attr : node->outputAttributes)
					{
						if (attr.id == from)
						{
							link->pOutput = &attr;
							link->pOutput->pLinks.push_back(link);
						}
					}

					for (auto& attr : node->inputAttributes)
					{
						if (attr.id == to)
						{
							link->pInput = &attr;
							link->pInput->pLinks.push_back(link);
						}
					}

					if (link->pInput && link->pOutput)
					{
						link->pInput->data = link->pOutput->data;
					}
				}
				graphSpec.links.push_back(link);
			}

			Ref<GraphKeyGraph> graph = CreateRef<GraphKeyGraph>(graphSpec);
			graph->SetCurrentId(currentId);

			entity->SetGraphKeyGraph(graph);
		}

		entities.emplace(entity->GetID(), entity);
	}

	void LevelLoader::DeserializeBrush(const YAML::Node& node, std::map<uint32_t, Brush*>& brushes) const
	{
		AssetHandle meshHandle = node["meshHandle"].as<AssetHandle>();
		Brush* brush = Brush::Create(g_pEnv->pAssetManager->GetPathFromAssetHandle(meshHandle).string());

		glm::vec3 pos = node["position"].as<glm::vec3>();
		brush->SetPosition(pos);

		glm::vec3 rot = node["rotation"].as<glm::vec3>();
		brush->SetRotation(rot);

		glm::vec3 scale = node["scale"].as<glm::vec3>();
		brush->SetScale(scale);

		uint32_t layerId = node["layerId"].as<uint32_t>();
		brush->SetLayerID(layerId);

		std::string name = node["brush"].as<std::string>();
		brush->SetName(name);

		brushes.emplace(std::make_pair(brush->GetID(), brush));
	}

	void LevelLoader::DeserializeAttribute(const YAML::Node& yamlNode, Ref<Node> node, uint32_t& currentId) const
	{
		std::string attrType = yamlNode["attrType"].as<std::string>();
		std::string name = yamlNode["name"].as<std::string>();

		if (attrType == "input")
		{
			for (auto& attr : node->inputAttributes)
			{
				if (attr.name == name)
				{
					LP_DESERIALIZE_PROPERTY(id, attr.id, yamlNode, 0);
					if (attr.id >= currentId)
					{
						currentId = attr.id + 1;
					}

					switch (attr.type)
					{
						case PropertyType::String: attr.data = std::any(yamlNode["data"].as<std::string>()); break;
						case PropertyType::Path: attr.data = std::any(yamlNode["data"].as<std::string>()); break;
						case PropertyType::Bool: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, false); break;
						case PropertyType::Int: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, 0); break;
						case PropertyType::Float: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, 0.f); break;
						case PropertyType::Float2: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, glm::vec2(0.f)); break;
						case PropertyType::Float3: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, glm::vec3(0.f)); break;
						case PropertyType::Float4: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, glm::vec4(0.f)); break;
						case PropertyType::Color3: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, glm::vec3(0.f)); break;
						case PropertyType::Color4: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, glm::vec4(0.f)); break;
						case PropertyType::EntityId: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, uint32_t(0)); break;
					
						default: break;
					}
				}
			}
		}
		else if (attrType == "output")
		{
			for (auto& attr : node->outputAttributes)
			{
				if (attr.name == name)
				{
					LP_DESERIALIZE_PROPERTY(id, attr.id, yamlNode, 0);
					if (attr.id >= currentId)
					{
						currentId = attr.id + 1;
					}

					switch (attr.type)
					{
						case PropertyType::String: attr.data = std::any(yamlNode["data"].as<std::string>()); break;
						case PropertyType::Path: attr.data = std::any(yamlNode["data"].as<std::string>()); break;
						case PropertyType::Bool: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, false); break;
						case PropertyType::Int: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, 0); break;
						case PropertyType::Float: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, 0.f); break;
						case PropertyType::Float2: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, glm::vec2(0.f)); break;
						case PropertyType::Float3: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, glm::vec3(0.f)); break;
						case PropertyType::Float4: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, glm::vec4(0.f)); break;
						case PropertyType::Color3: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, glm::vec3(0.f)); break;
						case PropertyType::Color4: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, glm::vec4(0.f)); break;
						case PropertyType::EntityId: LP_DESERIALIZE_PROPERTY_ANY(data, attr.data, yamlNode, uint32_t(0)); break;

						default: break;
					}
				}
			}
		}
	}

	void LevelLoader::SerializeAttribute(const Attribute& attr, const std::string& type, YAML::Emitter& out) const
	{
		out << YAML::BeginMap;

		LP_SERIALIZE_PROPERTY(name, attr.name, out);
		LP_SERIALIZE_PROPERTY(attrType, type, out);
		LP_SERIALIZE_PROPERTY(id, attr.id, out);
		LP_SERIALIZE_PROPERTY(propType, attr.type, out);


		switch (attr.type)
		{
			case Lamp::PropertyType::String: LP_SERIALIZE_PROPERTY(data, std::any_cast<std::string>(attr.data), out); break;
			case Lamp::PropertyType::Path: LP_SERIALIZE_PROPERTY(data, std::any_cast<std::filesystem::path>(attr.data).string(), out); break;
			case Lamp::PropertyType::Bool: LP_SERIALIZE_PROPERTY(data, std::any_cast<bool>(attr.data), out); break;
			case Lamp::PropertyType::Int: LP_SERIALIZE_PROPERTY(data, std::any_cast<int>(attr.data), out); break;
			case Lamp::PropertyType::Float: LP_SERIALIZE_PROPERTY(data, std::any_cast<float>(attr.data), out); break;
			case Lamp::PropertyType::Float2: LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec2>(attr.data), out); break;
			case Lamp::PropertyType::Float3: LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec3>(attr.data), out); break;
			case Lamp::PropertyType::Float4: LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec4>(attr.data), out); break;
			case Lamp::PropertyType::Color3: LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec3>(attr.data), out); break;
			case Lamp::PropertyType::Color4: LP_SERIALIZE_PROPERTY(data, std::any_cast<glm::vec4>(attr.data), out); break;
			case Lamp::PropertyType::Void: LP_SERIALIZE_PROPERTY(data, "", out); break;
			case Lamp::PropertyType::Selectable: LP_SERIALIZE_PROPERTY(data, "", out); break;
			case Lamp::PropertyType::EntityId: LP_SERIALIZE_PROPERTY(data, std::any_cast<uint32_t>(attr.data), out); break;
		}

		out << YAML::EndMap;
	}
}