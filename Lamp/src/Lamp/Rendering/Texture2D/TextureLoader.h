#pragma once

namespace Lamp {
class TextureLoader {
public:
  static std::tuple<uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>
  LoadTexture(const std::string &path);
};
} // namespace Lamp