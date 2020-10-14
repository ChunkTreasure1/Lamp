#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/Meshes/Model.h"
#include "Lamp/Rendering/LightBase.h"

namespace Lamp
{
	class LightComponent final : public EntityComponent
	{
	public:

		LightComponent()
			: EntityComponent("LightComponent")
		{
			SetComponentProperties
			({
				{ PropertyType::Float, "Light constant", RegisterData(&m_PointLight.LightConstant) },
				{ PropertyType::Float, "Linear constant", RegisterData(&m_PointLight.LinearConstant) },
				{ PropertyType::Float, "Quadratic constant", RegisterData(&m_PointLight.QuadraticConstant) },
				{ PropertyType::Float3, "Diffuse", RegisterData(&m_PointLight.Diffuse) },
				{ PropertyType::Float3, "Specular", RegisterData(&m_PointLight.Specular) }
			});
		}

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint64_t GetEventMask() override { return EventType::None; }
		////////////////

		//Getting
		inline const float GetLightConstant() { return m_PointLight.LightConstant; }
		inline const float GetLinearConstant() { return m_PointLight.LinearConstant; }
		inline const float GetQuadraticConstant() { return m_PointLight.QuadraticConstant; }

		inline const glm::vec3& GetDiffuse() { return m_PointLight.Diffuse; }
		inline const glm::vec3& GetSpecular() { return m_PointLight.Specular; }

		//Setting
		inline void SetLightConstant(float val) { m_PointLight.LightConstant = val; }
		inline void SetLinearConstant(float val) { m_PointLight.LinearConstant = val; }
		inline void SetQuadraticConstant(float val) { m_PointLight.QuadraticConstant = val; }

		inline void SetDiffuse(const glm::vec3& val) { m_PointLight.Diffuse = val; }
		inline void SetSpecular(const glm::vec3& val) { m_PointLight.Specular = val; }

	private:
		bool OnRender(AppRenderEvent& e);
		bool OnUpdate(AppUpdateEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<LightComponent>(); }
		static std::string GetFactoryName() { return "LightComponent"; }

	private:
		static bool s_Registered;

	private:
		PointLight m_PointLight;
	};
}