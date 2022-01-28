#pragma once

#include "ObjectLayer.h"
#include "Lamp/AssetSystem/Asset.h"
#include "Lamp/Event/Event.h"
#include "Lamp/Core/Time/Timestep.h"

#include "Lamp/Objects/Entity/BaseComponents/MeshComponent.h"
#include "Lamp/Event/EditorEvent.h"
#include "Lamp/Rendering/Skybox.h"
#include "Lamp/World/Terrain.h"

#include <string>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

namespace Lamp
{
	struct DirectionalLight;
	struct PointLight;
	class Brush;
	class Entity;
	class CameraBase;
	class Terrain;

	class RenderUtils
	{
	public:
		RenderUtils() {}
		~RenderUtils();

		void RegisterPointLight(PointLight* light);
		bool UnregisterPointLight(PointLight* light);

		void RegisterDirectionalLight(DirectionalLight* light);
		bool UnregisterDirectionalLight(DirectionalLight* light);

		void RegisterMeshComponent(uint32_t id, MeshComponent* mesh);
		void UnegisterMeshComponent(uint32_t id);

		inline const std::vector<PointLight*>& GetPointLights() { return m_PointLights; }
		inline const std::vector<DirectionalLight*>& GetDirectionalLights() { return m_DirectionalLights; }
		inline const std::unordered_map<uint32_t, MeshComponent*>& GetMeshComponents() { return m_MeshComponents; }

	private:
		std::vector<Lamp::PointLight*> m_PointLights;
		std::vector<Lamp::DirectionalLight*> m_DirectionalLights;
		
		std::unordered_map<uint32_t, MeshComponent*> m_MeshComponents;
	};

	struct LevelEnvironment
	{
		glm::vec3 CameraPosition{ 0.f, 0.f, 0.f };
		glm::quat CameraRotation{ 0.f, 0.f, 0.f, 0.f };

		float CameraFOV = 60.f;

		float terrainScale = 64.f;
		float terrainShift = 16.f;
	};

	class Level : public Asset
	{
	public:
		Level(const std::string& name);
		Level();

		Level(const Level& level);
		~Level();

		void OnEvent(Event& e);
		bool OnRenderEvent(AppRenderEvent& e);

		void UpdateEditor(Timestep ts, Ref<CameraBase>& camera);
		void UpdateSimulation(Timestep ts, Ref<CameraBase>& camera);
		void UpdateRuntime(Timestep ts);

		void Shutdown();

		void OnRuntimeStart();
		void OnRuntimeEnd();

		void OnSimulationStart();
		void OnSimulationEnd();

		//Layers
		void AddLayer(const ObjectLayer& layer);
		void RemoveLayer(uint32_t id);
		void MoveObjectToLayer(uint32_t currLayer, uint32_t newLayer, uint32_t objId);
		void AddToLayer(Object* obj);
		void RemoveFromLayer(Object* obj);

		inline LevelEnvironment& GetEnvironment() { return m_Environment; }
		inline const std::string& GetName() { return m_name; }
		inline std::map<uint32_t, Brush*>& GetBrushes() { return m_Brushes; }
		
		inline std::map<uint32_t, Entity*>& GetEntities() { return m_Entities; }
		inline std::vector<ObjectLayer>& GetLayers() { return m_Layers; }
		inline bool GetIsPlaying() { return m_IsPlaying; }
		
		inline RenderUtils& GetRenderUtils() { return m_RenderUtils; }
		inline Ref<Skybox> GetSkybox() { return m_skybox; }
		inline Ref<Terrain> GetTerrain() { return m_terrain; }

		inline void SetIsPlaying(bool playing) { m_IsPlaying = playing; }
		inline void SetSkybox(const std::filesystem::path& path) { m_skybox = Skybox::Create(path); }
		inline void SetTerrain(Ref<Terrain> terrain) { m_terrain = terrain; }

		static AssetType GetStaticType() { return AssetType::Level; }
		AssetType GetType() override { return GetStaticType(); }


	private:
		friend class LevelLoader;

		void RenderLevel(Ref<CameraBase> camera);
		bool OnViewportResize(EditorViewportSizeChangedEvent& e);

		std::string m_name;
		bool m_IsPlaying = false;
		bool m_LastShowedGizmos = false;

		LevelEnvironment m_Environment;
		RenderUtils m_RenderUtils;

		Ref<Skybox> m_skybox;
		Ref<Terrain> m_terrain;

		std::map<uint32_t, Brush*> m_Brushes;
		std::map<uint32_t, Entity*> m_Entities;

		std::vector<ObjectLayer> m_Layers;
	};
}