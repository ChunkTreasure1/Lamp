#pragma once

#include "ObjectLayer.h"

#include "Lamp/AssetSystem/Asset.h"
#include "Lamp/Core/Time/Timestep.h"

#include "Lamp/Objects/Entity/BaseComponents/MeshComponent.h"

#include "Lamp/Event/EditorEvent.h"
#include "Lamp/Event/Event.h"

#include "Lamp/World/Terrain.h"
#include "Lamp/Level/Environment.h"

#include "Lamp/Rendering/RenderPass.h"
#include "Lamp/Rendering/Skybox.h"

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

		inline Environment& GetEnvironment() { return m_environment; }
		inline const std::string& GetName() { return m_name; }
		inline std::map<uint32_t, Brush*>& GetBrushes() { return m_brushes; }
		
		inline std::map<uint32_t, Entity*>& GetEntities() { return m_entities; }
		inline std::vector<ObjectLayer>& GetLayers() { return m_layers; }
		inline bool IsPlaying() const { return m_isPlaying; }

		inline const Ref<Image2D> GetFinalRenderedImage() const { return m_renderPasses.back().graphicsPipeline->GetSpecification().framebuffer->GetColorAttachment(0); }
		inline const Ref<Framebuffer> GetGeometryFramebuffer() const { return m_geometryFramebuffer; }

		inline const bool HasTerrain() const { return m_environment.GetTerrain().terrain != nullptr && m_environment.GetTerrain().terrain->IsValid(); }
		inline const bool HasSkybox() const { return m_environment.GetSkybox().skybox != nullptr && !m_environment.GetSkybox().skybox->IsFlagSet(AssetFlag::Missing) && !m_environment.GetSkybox().skybox->IsFlagSet(AssetFlag::Invalid); }

		inline void SetIsPlaying(bool state) { m_isPlaying = state; }
		inline void SetRenderPasses(const std::vector<RenderPass>& passes) { m_renderPasses = passes; }

		static AssetType GetStaticType() { return AssetType::Level; }
		AssetType GetType() override { return GetStaticType(); }

	private:
		friend class LevelLoader;

		void SetupRenderPasses();
		void RenderLevel(const Ref<CameraBase> camera);

		bool OnViewportResize(EditorViewportSizeChangedEvent& e);

		std::string m_name;
		bool m_isPlaying = false;

		Environment m_environment;

		std::map<uint32_t, Brush*> m_brushes;
		std::map<uint32_t, Entity*> m_entities;

		std::vector<ObjectLayer> m_layers;
		std::vector<RenderPass> m_renderPasses;

		//Move from here
		bool m_lastShowedGizmos = false;
		Ref<Framebuffer> m_geometryFramebuffer;
	};
}