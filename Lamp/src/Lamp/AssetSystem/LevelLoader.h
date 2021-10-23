#pragma once

#include "AssetLoader.h"
#include <string>

#include "Lamp/Objects/Entity/Base/ComponentProperties.h"

namespace YAML {
class Emitter;
}

namespace Lamp {
class Brush;
class Entity;
class LevelLoader : public AssetLoader {
public:
  virtual void Save(const Ref<Asset> &asset) const override;
  virtual bool Load(const std::filesystem::path &path,
                    Ref<Asset> &asset) const override;

private:
  void SaveEntity(YAML::Emitter &out, const Entity *pEnt) const;
  void SaveBrush(YAML::Emitter &out, const Brush *pBrush) const;

  template <typename T>
  T *GetPropertyData(const std::string &name,
                     const std::vector<ComponentProperty> &properties) const {
    for (const auto &prop : properties) {
      if (prop.name == name) {
        T *p = static_cast<T *>(prop.value);
        return p;
      }
    }

    return nullptr;
  }
};
} // namespace Lamp
