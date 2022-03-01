#pragma once

#include "Lamp/Core/Core.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>

namespace Lamp
{
	struct PointLight;
	struct DirectionalLight;
	class Skybox;
	class Terrain;

	struct SkyboxData
	{
		float environmentLod = 1.f;
		float environmentMultiplier = 1.f;
		float hdrExposure = 1.f;
		float ambianceMultiplier = 0.5f;
	
		Ref<Skybox> skybox;
	};

	struct TerrainData
	{
		float terrainScale = 64.f;
		float terrainShift = 16.f;
		Ref<Terrain> terrain;
	};

	class Environment
	{
	public:
		Environment() = default;
		~Environment();

		void RegisterPointLight(PointLight* light);
		bool UnregisterPointLight(PointLight* light);

		void RegisterDirectionalLight(DirectionalLight* light);
		bool UnregisterDirectionalLight(DirectionalLight* light);

		inline const std::vector<PointLight*>& GetPointLights() { return m_pointLights; }
		inline const std::vector<DirectionalLight*>& GetDirectionalLights() { return m_directionalLights; }

		inline const glm::vec3& GetCameraPosition() const { return m_cameraPosition; } //TODO: move from here
		inline const glm::quat& GetCameraRotation() const { return m_cameraRotation; }
		inline const float& GetCameraFOV() const { return m_cameraFOV; }

		inline void SetCameraPosition(const glm::vec3& aPos) { m_cameraPosition = aPos; }
		inline void SetCameraRotation(const glm::quat& aRot) { m_cameraRotation = aRot; }

		inline const SkyboxData& GetSkybox() const { return m_skyboxData; }
		inline const TerrainData& GetTerrain() const { return m_terrainData; }

	private:
		friend class Level;

		//Editor
		glm::vec3 m_cameraPosition{ 0.f, 0.f, 0.f };
		glm::quat m_cameraRotation{ 0.f, 0.f, 0.f, 0.f };
		float m_cameraFOV = 60.f;

		std::vector<Lamp::PointLight*> m_pointLights;
		std::vector<Lamp::DirectionalLight*> m_directionalLights;

		SkyboxData m_skyboxData;
		TerrainData m_terrainData;
	};
}