#pragma once

#include <string>
#include "AssetLoader.h"

#include "Lamp/Objects/Entity/Base/ComponentProperties.h"

namespace YAML
{
	class Emitter;
	class Node;
}

namespace Lamp
{
	class Brush;
	class Entity;
	class Attribute;
	struct Node;

	class LevelLoader : public AssetLoader
	{
	public:
		void Save(const Ref<Asset>& asset) const override;
		bool Load(const std::filesystem::path& path, Asset* asset) const override;
	
	private:
		void SerializeEntity(YAML::Emitter& out, const Entity* pEnt) const;
		void SerializeBrush(YAML::Emitter& out, const Brush* pBrush) const;
		void SerializeAttribute(const Attribute& attr, const std::string& type, YAML::Emitter& out) const;

		void DeserializeEntity(const YAML::Node& node, std::map<uint32_t, Entity*>& entities, Level* level) const;
		void DeserializeBrush(const YAML::Node& node, std::map<uint32_t, Brush*>& brushes, Level* level) const;
		void DeserializeAttribute(const YAML::Node& yamlNode, Ref<Node> node, uint32_t& currentId) const;

		template<typename T> T* GetPropertyData(const std::string& name, const std::vector<ComponentProperty>& properties) const;
	};

	template<typename T>
	inline T* LevelLoader::GetPropertyData(const std::string& name, const std::vector<ComponentProperty>& properties) const
	{
		for (const auto& prop : properties)
		{
			if (prop.name == name)
			{
				T* p = static_cast<T*>(prop.value);
				return p;
			}
		}

		return nullptr;
	}
}