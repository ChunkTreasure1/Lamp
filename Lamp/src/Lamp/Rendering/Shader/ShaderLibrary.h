#pragma once

#include "Lamp/Core/Core.h"
#include "Shader.h"
#include <vector>

namespace Lamp {
class ShaderLibrary {
public:
  static void AddShader(const std::string &vertexPath,
                        const std::string &fragmentPath,
                        const std::string &geoPath = "");
  static void LoadShaders();

  static Ref<Shader> &GetShader(const std::string &name);
  static std::vector<Ref<Shader>> &GetShaders() { return m_Shaders; }

private:
  ShaderLibrary() = delete;

private:
  static std::vector<Ref<Shader>> m_Shaders;
};
} // namespace Lamp
