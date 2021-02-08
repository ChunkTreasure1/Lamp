#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Lamp/Rendering/LightBase.h"

namespace Lamp {
class ObjectLayerManager;
class EntityManager;
class BrushManager;
} // namespace Lamp

class RenderUtils {
public:
  RenderUtils() {}
  ~RenderUtils();

  void RegisterPointLight(Lamp::PointLight *light);
  bool UnregisterPointLight(Lamp::PointLight *light);

  inline const std::vector<Lamp::PointLight *> &GetPointLights() {
    return m_PointLights;
  }

private:
  std::vector<Lamp::PointLight *> m_PointLights;
};

struct GlobalEnvironment {
  Lamp::DirectionalLight DirLight;
  bool ShouldRenderBB = false;
  bool ShouldRenderGizmos = true;

  RenderUtils *pRenderUtils = nullptr;

  Lamp::ObjectLayerManager *pObjectLayerManager;
  Lamp::EntityManager *pEntityManager;
  Lamp::BrushManager *pBrushManager;
};

struct SAABB {
  glm::vec3 Max = glm::vec3(0.f);
  glm::vec3 Min = glm::vec3(0.f);
};
