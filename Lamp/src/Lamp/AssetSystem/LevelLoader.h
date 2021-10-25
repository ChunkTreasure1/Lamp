#pragma once

#include <string>
#include "AssetLoader.h"

#include "Lamp/Objects/Entity/Base/ComponentProperties.h"

namespace YAML
{
	class Emitter;
}

namespace Lamp
{
	class Brush;
	class Entity;
	class Attribute;

	class LevelLoader : public AssetLoader
	{
	public:
		virtual void Save(const Ref<Asset>& asset) const override;
		virtual bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
	
	private:
		void SerializeEntity(YAML::Emitter& out, const Entity* pEnt) const;
		void SerializeBrush(YAML::Emitter& out, const Brush* pBrush) const;

		void SerializeAttribute(const Attribute& attr, const std::string& type, YAML::Emitter& out) const;

		template<typename T>
		T* GetPropertyData(const std::string& name, const std::vector<ComponentProperty>& properties) const
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
	};
}