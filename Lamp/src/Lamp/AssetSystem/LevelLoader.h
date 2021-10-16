#pragma once

#include <string>
#include "AssetLoader.h"

#include "Lamp/Objects/Entity/Base/ComponentProperties.h"

namespace Lamp
{
	class LevelLoader : public AssetLoader
	{
	public:
		virtual void Save(const Ref<Asset>& asset) const override;
		virtual bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
	
	private:
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