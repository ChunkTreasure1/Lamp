#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/Meshes/Model.h"

namespace Lamp
{
	class LightComponent final : public EntityComponent
	{
	public:

		LightComponent()
			: EntityComponent("LightComponent"), m_Ambient({ 0.f, 0.f, 0.f }), m_Diffuse({ 0.f, 0.f, 0.f }),
			m_Specular({ 0.f, 0.f, 0.f })
		{
			SetComponentProperties
			({
				{ PropertyType::Float, "Light constant", RegisterData(&m_LightConstant) },
				{ PropertyType::Float, "Linear constant", RegisterData(&m_LinearConstant) },
				{ PropertyType::Float, "Quadratic constant", RegisterData(&m_QuadraticConstant) },
				{ PropertyType::Float3, "Ambient", RegisterData(&m_Ambient) },
				{ PropertyType::Float3, "Diffuse", RegisterData(&m_Diffuse) },
				{ PropertyType::Float3, "Specular", RegisterData(&m_Specular) }
			});
		}

		//Getting
		inline const float GetLightConstant() { return m_LightConstant; }
		inline const float GetLinearConstant() { return m_LightConstant; }
		inline const float GetQuadraticConstant() { return m_QuadraticConstant; }

		inline const glm::vec3& GetAmbient() { return m_Ambient; }
		inline const glm::vec3& GetDiffuse() { return m_Diffuse; }
		inline const glm::vec3& GetSpecular() { return m_Specular; }

		//Setting
		inline void SetLightConstant(float val) { m_LightConstant = val; }
		inline void SetLinearConstant(float val) { m_LinearConstant = val; }
		inline void SetQuadraticConstant(float val) { m_QuadraticConstant = val; }

		inline void SetAmbient(const glm::vec3& val) { m_Ambient = val; }
		inline void SetDiffuse(const glm::vec3& val) { m_Diffuse = val; }
		inline void SetSpecular(const glm::vec3& val) { m_Specular = val; }

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		////////////////

	private:
		bool OnRender(AppRenderEvent& e);
		bool OnUpdate(AppUpdateEvent& e);

	public:
		static Ref<EntityComponent> Create() { return std::make_shared<LightComponent>(); }
		static std::string GetFactoryName() { return "LightComponent"; }

	private:
		static bool s_Registered;

	private:
		float m_LightConstant = 1.f;
		float m_LinearConstant = 0.09f;
		float m_QuadraticConstant = 0.032f;

		glm::vec3 m_Ambient;
		glm::vec3 m_Diffuse;
		glm::vec3 m_Specular;
	};
}