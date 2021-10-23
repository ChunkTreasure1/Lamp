#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace Lamp {
class Object;
struct ObjectLayer {
  ObjectLayer(const std::string &name, uint32_t id, bool active)
      : Name(name), ID(id), Active(active) {}

  ~ObjectLayer() = default;

  std::string Name;
  uint32_t ID;
  bool Active;

  std::vector<Object *> Objects;
};
} // namespace Lamp