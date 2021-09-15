#pragma once

#include <string>
#include "Lamp/Objects/Brushes/Brush.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Layer.h"
#include "Lamp/Objects/Entity/BaseComponents/MeshComponent.h"

namespace Lamp
{
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

	struct LevelEnvironment
	{
		glm::vec3 GlobalAmbient{ 0.3f, 0.3f, 0.3f };
		glm::vec3 CameraPosition{ 0.f, 0.f, 0.f };
		glm::vec3 CameraRotation{ 0.f, 0.f, 0.f };

		float CameraFOV = 60.f;
	};

	struct LevelLoadData
	{
		std::string name;
		std::string path;

		LevelEnvironment env;
	};

	class Level
	{
	public:
		Level(const std::string& name, const std::string& path)
			: m_Name(name), m_Path(path)
		{
		}

		Level(const Level& level);

		~Level()
		{
		}

		inline LevelEnvironment& GetEnvironment() { return m_Environment; }
		inline const std::string& GetName() { return m_Name; }
		inline const std::string& GetPath() { return m_Path; }
		inline std::map<uint32_t, Brush*>& GetBrushes() { return m_Brushes;  }
		inline std::map<uint32_t, Entity*>& GetEntities() { return m_Entities;  }
		inline std::vector<ObjectLayer>& GetLayers() { return m_Layers;  }
		inline bool GetIsPlaying() { return m_IsPlaying; }
		inline RenderUtils& GetRenderUtils() { return m_RenderUtils; }

		inline void SetPath(const std::string& path) { m_Path = path; }
		inline void SetIsPlaying(bool playing) { m_IsPlaying = playing; }

		void OnEvent(Event& e);
		void UpdateEditor(Timestep ts, Ref<CameraBase>& camera);
		void UpdateRuntime(Timestep ts);

		void OnRuntimeStart();
		void OnRuntimeEnd();

		void SetupLights();

	private:
		std::string m_Name;
		std::string m_Path;
		LevelEnvironment m_Environment;
		bool m_IsPlaying = false;
		bool m_LastShowedGizmos = false;

		std::map<uint32_t, Brush*> m_Brushes;
		std::map<uint32_t, Entity*> m_Entities;
		std::vector<ObjectLayer> m_Layers;
		RenderUtils m_RenderUtils;
	};
}