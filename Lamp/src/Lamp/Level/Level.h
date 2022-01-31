#pragma once

#include "ObjectLayer.h"

#include "Lamp/AssetSystem/Asset.h"
#include "Lamp/Core/Time/Timestep.h"

#include "Lamp/Objects/Entity/BaseComponents/MeshComponent.h"

#include "Lamp/Event/EditorEvent.h"
#include "Lamp/Event/Event.h"

#include "Lamp/World/Terrain.h"

#include "Lamp/Rendering/RenderPass.h"
#include "Lamp/Rendering/Skybox.h"

#include <glm/gtc/quaternion.hpp>

#include <string>
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

		void UpdateEditor(Timestep ts);
		void UpdateSimulation(Timestep ts);
		void UpdateRuntime(Timestep ts);

		void RenderEditor(const Ref<CameraBase> camera);
		void RenderSimulation(const Ref<CameraBase> camera);
		void RenderRuntime();

		void Shutdown();

		void OnRuntimeStart();
		void OnRuntimeEnd();

		void OnSimulationStart();
		void OnSimulationEnd();

		//Layers
		void AddLayer(const ObjectLayer& layer);
		void AddToLayer(Object* obj);

		void MoveObjectToLayer(uint32_t currLayer, uint32_t newLayer, uint32_t objId);
		
		void RemoveLayer(uint32_t id);
		void RemoveFromLayer(Object* obj);

		inline LevelEnvironment& GetEnvironment() { return m_environment; }
		inline const std::string& GetName() { return m_name; }
		inline std::map<uint32_t, Brush*>& GetBrushes() { return m_Brushes; }
		
		inline std::map<uint32_t, Entity*>& GetEntities() { return m_Entities; }
		inline std::vector<ObjectLayer>& GetLayers() { return m_layers; }
		inline bool IsPlaying() const { return m_isPlaying; }

		inline RenderUtils& GetRenderUtils() { return m_renderUtils; }
		inline Ref<Skybox> GetSkybox() { return m_skybox; }
		inline Ref<Terrain> GetTerrain() { return m_terrain; }

		inline void SetIsPlaying(bool state) { m_isPlaying = state; }
		inline void SetSkybox(const std::filesystem::path& path) { m_skybox = Skybox::Create(path); }
		inline void SetTerrain(Ref<Terrain> terrain) { m_terrain = terrain; }
		inline void SetRenderPasses(const std::vector<RenderPass>& passes) { m_renderPasses = passes; }

		static AssetType GetStaticType() { return AssetType::Level; }
		AssetType GetType() override { return GetStaticType(); }


	private:
		friend class LevelLoader;

		void RenderLevel(const Ref<CameraBase> camera);
		bool OnViewportResize(EditorViewportSizeChangedEvent& e);

		std::string m_name;
		bool m_isPlaying = false;

		//Merge these two
		LevelEnvironment m_environment;
		RenderUtils m_renderUtils;

		Ref<Skybox> m_skybox;
		Ref<Terrain> m_terrain;

		std::map<uint32_t, Brush*> m_Brushes;
		std::map<uint32_t, Entity*> m_Entities;

		std::vector<ObjectLayer> m_layers;

		//Move from here
		bool m_lastShowedGizmos = false;
		std::vector<RenderPass> m_renderPasses;
	};
}