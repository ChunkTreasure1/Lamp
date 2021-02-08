#pragma once

#include "Lamp/Event/Event.h"
#include "Object.h"

#include <string>

namespace Lamp {
struct ObjectLayer {
  ObjectLayer(uint32_t num, const std::string &name, bool state)
      : ID(num), Name(name), IsDestroyable(state), IsActive(true) {}

  ObjectLayer(uint32_t num) : ID(num), Name("Layer"), IsDestroyable(true) {}

  ~ObjectLayer() {
    for (int i = 0; i < Objects.size(); i++) {
      Objects[i]->Destroy();

      // Needs to be done as Object::Destroy removes it self from the object
      // layer
      i--;
    }

    Objects.clear();
  }

  uint32_t ID;
  std::string Name;
  std::vector<Object *> Objects;
  bool IsDestroyable;
  bool IsActive;
};

class ObjectLayerManager {
public:
  ObjectLayerManager();

  ~ObjectLayerManager() { m_Layers.clear(); }

  void OnEvent(Event &e);
  void Destroy();

  inline void AddLayer(ObjectLayer &layer) { m_Layers.push_back(layer); }
  inline bool RemoveLayer(uint32_t id);
  inline std::vector<ObjectLayer> &GetLayers() { return m_Layers; }
  inline void SetLayers(std::vector<ObjectLayer> &layers) { m_Layers = layers; }

  void AddToLayer(Object *obj, uint32_t layerId);
  void AddToLayer(Object *obj, const std::string &name);

  void RemoveFromLayer(Object *obj, uint32_t layerId);
  bool Remove(Object *obj);

  void MoveToLayer(Object *obj, uint32_t layerId);

  Object *GetObjectFromPoint(const glm::vec3 &pos, const glm::vec3 &origin);
  Object *GetObjectFromId(uint32_t id);

public:
  static ObjectLayerManager *Get() { return s_ObjectLayerManager; }

private:
  bool Exists(uint32_t layerId) {
    for (auto &layer : m_Layers) {
      if (layer.ID == layerId) {
        return true;
      }
    }

    return false;
  }

private:
  static ObjectLayerManager *s_ObjectLayerManager;

private:
  std::vector<ObjectLayer> m_Layers;
};
} // namespace Lamp
