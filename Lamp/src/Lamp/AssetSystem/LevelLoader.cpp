#include "lppch.h"
#include "LevelLoader.h"

#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Level/Level.h"
#include "AssetManager.h"

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

			out << YAML::Key << "brushes" << YAML::BeginSeq;
			for (const auto& brush : level->GetBrushes())
			{
				out << YAML::BeginMap;
				out << YAML::Key << "brush" << YAML::Value << brush.second->GetName();
				out << YAML::Key << "meshHandle" << YAML::Value << brush.second->GetModel()->Handle;
				out << YAML::Key << "position" << YAML::Value << brush.second->GetPosition();
				out << YAML::Key << "rotation" << YAML::Value << brush.second->GetRotation();
				out << YAML::Key << "scale" << YAML::Value << brush.second->GetScale();
				out << YAML::Key << "layerId" << YAML::Value << brush.second->GetLayerID();
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "entities" << YAML::BeginSeq;
			for (const auto& entityPair : level->GetEntities())
			{
				if (entityPair.second == nullptr)
				{
					continue;
				}

				auto entity = entityPair.second;
				if (!entity->GetSaveable())
				{
					continue;
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
						LP_SERIALIZE_PROPERTY(param, prop.Name, out);
						LP_SERIALIZE_PROPERTY(type, ComponentProperties::GetStringFromType(prop.PropertyType), out);
						
						switch (prop.PropertyType)
						{
							case PropertyType::String:
								LP_SERIALIZE_PROPERTY(value, *static_cast<std::string*>(prop.Value), out);
								break;

							case PropertyType::Bool:
								LP_SERIALIZE_PROPERTY(value, *static_cast<bool*>(prop.Value), out);
								break;

							case PropertyType::Int:
								LP_SERIALIZE_PROPERTY(value, *static_cast<int*>(prop.Value), out);
								break;

							case PropertyType::Float:
								LP_SERIALIZE_PROPERTY(value, *static_cast<float*>(prop.Value), out);
								break;

							case PropertyType::Float2:
								LP_SERIALIZE_PROPERTY(value, *static_cast<glm::vec2*>(prop.Value), out);
								break;

							case PropertyType::Float3:
								LP_SERIALIZE_PROPERTY(value, *static_cast<glm::vec3*>(prop.Value), out);
								break;

							case PropertyType::Float4:
								LP_SERIALIZE_PROPERTY(value, *static_cast<glm::vec4*>(prop.Value), out);
								break;

							case PropertyType::Path:
								LP_SERIALIZE_PROPERTY(value, *static_cast<std::string*>(prop.Value), out);
								break;

							case PropertyType::Color3:
								LP_SERIALIZE_PROPERTY(value, *static_cast<glm::vec3*>(prop.Value), out);
								break;

							case PropertyType::Color4:
								LP_SERIALIZE_PROPERTY(value, *static_cast<glm::vec4*>(prop.Value), out);
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

				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

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

			out << YAML::Key << "LevelEnvironment" << YAML::Value;
			out << YAML::BeginMap;
			{

				LP_SERIALIZE_PROPERTY(cameraPosition, level->GetEnvironment().CameraPosition, out);
				LP_SERIALIZE_PROPERTY(cameraRotation, level->GetEnvironment().CameraRotation, out);
				LP_SERIALIZE_PROPERTY(cameraFOV, level->GetEnvironment().CameraFOV, out);
			}
			out << YAML::EndMap;

			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		std::ofstream file;
		file.open(asset->Path);
		file << out.c_str();
		file.close();
	}

	bool LevelLoader::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
	{
		asset = CreateRef<Level>();
		Ref<Level> level = std::dynamic_pointer_cast<Level>(asset);
		g_pEnv->pLevel = level;

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

		level->m_Name = levelNode["name"].as<std::string>();
		LP_DESERIALIZE_PROPERTY(handle, level->Handle, levelNode, (AssetHandle)0);

		//Brushes
		YAML::Node brushesNode = levelNode["brushes"];
		std::map<uint32_t, Brush*> brushes;
		
		for (auto entry : brushesNode)
		{
			AssetHandle meshHandle = entry["meshHandle"].as<AssetHandle>();
			Brush* brush = Brush::Create(g_pEnv->pAssetManager->GetPathFromAssetHandle(meshHandle).string());

			glm::vec3 pos = entry["position"].as<glm::vec3>();
			brush->SetPosition(pos);

			glm::vec3 rot = entry["rotation"].as<glm::vec3>();
			brush->SetRotation(rot);

			glm::vec3 scale = entry["scale"].as<glm::vec3>();
			brush->SetScale(scale);

			uint32_t layerId = entry["layerId"].as<uint32_t>();
			brush->SetLayerID(layerId);

			std::string name = entry["brush"].as<std::string>();
			brush->SetName(name);

			brushes.emplace(std::make_pair(brush->GetID(), brush));
		}

		//Entities
		YAML::Node entitiesNode = levelNode["entities"];
		std::map<uint32_t, Entity*> entities;

		for (auto entry : entitiesNode)
		{
			uint32_t layerId = entry["layerId"].as<uint32_t>();
			Entity* entity = Entity::Create(true, layerId);

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

					EntityPropertyChangedEvent e;
					pComp->OnEvent(e);
				}

				entity->AddComponent(pComp);
			}

			entities.emplace(entity->GetID(), entity);
		}

		//Layers
		YAML::Node layersNode = levelNode["layers"];
		std::vector<ObjectLayer> layers;

		for (auto entry : layersNode)
		{
			std::string name = entry["layer"].as<std::string>();
			uint32_t id = entry["id"].as<uint32_t>();
			bool active = entry["active"].as<bool>();

			layers.push_back(ObjectLayer(name, id, active));
		}

		level->m_Brushes = brushes;
		level->m_Entities = entities;
		level->m_Layers = layers;

		if (level->m_Layers.empty())
		{
			level->AddLayer(ObjectLayer("Main", 0, true));
		}

		YAML::Node envNode = levelNode["LevelEnvironment"];
		LP_DESERIALIZE_PROPERTY(cameraPosition, level->m_Environment.CameraPosition, envNode, glm::vec3(0.f));
		LP_DESERIALIZE_PROPERTY(cameraRotation, level->m_Environment.CameraRotation, envNode, glm::quat());
		LP_DESERIALIZE_PROPERTY(cameraFOV, level->m_Environment.CameraFOV, envNode, 60.f);

		level->Path = path;

		return true;
	}
}