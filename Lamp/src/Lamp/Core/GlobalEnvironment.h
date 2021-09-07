#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

#include "Lamp/Rendering/LightBase.h"

namespace Lamp
{
	class ObjectLayerManager;
	class EntityManager;
	class BrushManager;
	class AssetManager;
	class MeshComponent;
}

class RenderUtils
{
public:
	RenderUtils() {}
	~RenderUtils();

	void RegisterPointLight(Lamp::PointLight* light);
	bool UnregisterPointLight(Lamp::PointLight* light);

	void RegisterMeshComponent(uint32_t id, Lamp::MeshComponent* mesh);
	bool UnegisterMeshComponent(uint32_t id);

	inline const std::vector<Lamp::PointLight*>& GetPointLights() { return m_PointLights; }
	inline const std::unordered_map<uint32_t, Lamp::MeshComponent*>& GetMeshComponents() { return m_MeshComponents; }

private:
	std::vector<Lamp::PointLight*> m_PointLights;
	std::unordered_map<uint32_t, Lamp::MeshComponent*> m_MeshComponents;
};

struct GlobalEnvironment
{
	Lamp::DirectionalLight DirLight;
	bool ShouldRenderBB = false;
	bool ShouldRenderGizmos = true;

	RenderUtils* pRenderUtils = nullptr;

	Lamp::ObjectLayerManager* pObjectLayerManager = nullptr;
	Lamp::EntityManager* pEntityManager = nullptr;
	Lamp::BrushManager* pBrushManager = nullptr;
	Lamp::AssetManager* pAssetManager = nullptr;
};

struct SAABB
{
	glm::vec3 Max = glm::vec3(0.f);
	glm::vec3 Min = glm::vec3(0.f);

	glm::vec3 StartMax = glm::vec3(0.f);
	glm::vec3 StartMin = glm::vec3(0.f);
};