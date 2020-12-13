#pragma once

#include <glad/glad.h>

#include <string>

#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Lamp {
enum class ShaderType { Illum = 0, Blinn, Phong, BlinnPhong, Unknown };

struct ShaderSpec {
  std::string Name;
  int TextureCount;
  std::vector<std::string> TextureNames;
};

class Shader {
public:
  virtual ~Shader() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual void UploadBool(const std::string &name, bool value) const = 0;
  virtual void UploadInt(const std::string &name, int value) const = 0;
  virtual void UploadFloat(const std::string &name, float value) const = 0;
  virtual void UploadFloat3(const std::string &name,
                            const glm::vec3 &value) const = 0;
  virtual void UploadFloat4(const std::string &name,
                            const glm::vec4 &value) const = 0;

  virtual void UploadMat4(const std::string &name, const glm::mat4 &mat) = 0;
  virtual void UploadMat3(const std::string &name, const glm::mat3 &mat) = 0;
  virtual void UploadIntArray(const std::string &name, int *values,
                              uint32_t count) const = 0;

  virtual const std::string &GetName() = 0;
  virtual std::string &GetFragmentPath() = 0;
  virtual std::string &GetVertexPath() = 0;

  inline const ShaderType GetType() { return m_Type; }
  inline const ShaderSpec GetSpecifications() { return m_Specifications; }

public:
  static Ref<Shader> Create(const std::string &vertexPath,
                            const std::string &fragmentPath);

protected:
  ShaderType ShaderTypeFromString(const std::string &s);

protected:
  ShaderType m_Type;
  ShaderSpec m_Specifications;
};
} // namespace Lamp