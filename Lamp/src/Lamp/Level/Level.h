#pragma once

#include "Lamp/AssetSystem/Asset.h"
#include "Lamp/Core/Time/Timestep.h"
#include "Lamp/Event/Event.h"
#include "ObjectLayer.h"

#include "Lamp/Event/EditorEvent.h"
#include "Lamp/Objects/Entity/BaseComponents/MeshComponent.h"
#include "Lamp/Rendering/Skybox.h"

#include <glm/gtc/quaternion.hpp>
#include <string>
#include <unordered_map>

namespace Lamp {
struct DirectionalLight;
struct PointLight;
class Brush;
class Entity;
class CameraBase;

class RenderUtils {
public:
  RenderUtils() {}
  ~RenderUtils();

  void RegisterPointLight(PointLight *light);
  bool UnregisterPointLight(PointLight *light);

  void RegisterDirectionalLight(DirectionalLight *light);
  bool UnregisterDirectionalLight(DirectionalLight *light);

  void RegisterMeshComponent(uint32_t id, MeshComponent *mesh);
  bool UnegisterMeshComponent(uint32_t id);

  inline const std::vector<PointLight *> &GetPointLights() {
    return m_PointLights;
  }
  inline const std::vector<DirectionalLight *> &GetDirectionalLights() {
    return m_DirectionalLights;
  }
  inline const std::unordered_map<uint32_t, MeshComponent *> &
  GetMeshComponents() {
    return m_MeshComponents;
  }

private:
  std::vector<Lamp::PointLight *> m_PointLights;
  std::vector<Lamp::DirectionalLight *> m_DirectionalLights;

  std::unordered_map<uint32_t, MeshComponent *> m_MeshComponents;
};

struct LevelEnvironment {
  glm::vec3 CameraPosition{0.f, 0.f, 0.f};
  glm::quat CameraRotation;

  float CameraFOV = 60.f;
};

class Level : public Asset {
public:
  Level(const std::string &name) : m_Name(name) {}

  Level() {
    // Reserve 100 layer slots
    m_Layers.reserve(100);
  }

  Level(const Level &level);

  ~Level();

  inline LevelEnvironment &GetEnvironment() { return m_Environment; }
  inline const std::string &GetName() { return m_Name; }
  inline std::map<uint32_t, Brush *> &GetBrushes() { return m_Brushes; }
  inline std::map<uint32_t, Entity *> &GetEntities() { return m_Entities; }
  inline std::vector<ObjectLayer> &GetLayers() { return m_Layers; }
  inline bool GetIsPlaying() { return m_IsPlaying; }
  inline RenderUtils &GetRenderUtils() { return m_RenderUtils; }
  inline Ref<Skybox> GetSkybox() { return m_skybox; }

  inline void SetIsPlaying(bool playing) { m_IsPlaying = playing; }
  inline void SetSkybox(const std::filesystem::path &path) {
    m_skybox = Skybox::Create(path);
  }

  static AssetType GetStaticType() { return AssetType::Level; }
  AssetType GetType() override { return GetStaticType(); }

  void OnEvent(Event &e);
  void UpdateEditor(Timestep ts, Ref<CameraBase> &camera);
  void UpdateSimulation(Timestep ts, Ref<CameraBase> &camera);
  void UpdateRuntime(Timestep ts);

  void Shutdown();

  void OnRuntimeStart();
  void OnRuntimeEnd();

  void OnSimulationStart();
  void OnSimulationEnd();

  void SetupLights();

  // Layers
  void AddLayer(const ObjectLayer &layer);
  void RemoveLayer(uint32_t id);
  void MoveObjectToLayer(uint32_t currLayer, uint32_t newLayer, uint32_t objId);
  void AddToLayer(Object *obj);
  void RemoveFromLayer(Object *obj);

  friend class LevelLoader;

private:
  void RenderLevel(Ref<CameraBase> camera);
  bool OnViewportResize(EditorViewportSizeChangedEvent &e);

private:
  std::string m_Name;
  LevelEnvironment m_Environment;
  bool m_IsPlaying = false;
  bool m_LastShowedGizmos = false;

  Ref<Skybox> m_skybox;
  std::map<uint32_t, Brush *> m_Brushes;
  std::map<uint32_t, Entity *> m_Entities;
  std::vector<ObjectLayer> m_Layers;
  RenderUtils m_RenderUtils;
};
} // namespace Lamp
