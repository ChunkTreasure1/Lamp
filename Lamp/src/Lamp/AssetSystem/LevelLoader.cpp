#include "lppch.h"
#include "LevelLoader.h"

#include "Lamp/Utility/YAMLSerializationHelpers.h"
#include "Lamp/Utility/SerializeMacros.h"
#include "Lamp/Level/Level.h"

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

				LP_SERIALIZE_PROPERTY(globalAmbient, level->GetEnvironment().GlobalAmbient, out);
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
		return false;
	}
}