#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

namespace Lamp
{
	struct PointLight;
	class PointLightComponent final : public EntityComponent
	{
	public:

		PointLightComponent();
		~PointLightComponent();

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		////////////////

		//Getting
		inline const float GetIntensity() { return m_pPointLight->intensity; }
		inline const float GetRadius() { return m_pPointLight->radius; }
		inline const float GetFalloff() { return m_pPointLight->falloff; }

		inline const glm::vec3& GetColor() { return m_pPointLight->color; }
		inline const float GetFarPlane() { return m_pPointLight->farPlane; }
		inline PointLight* GetLight() { return m_pPointLight; }

		//Setting
		inline void SetIntensity(float val) { m_pPointLight->intensity = val; }
		inline void SetColor(const glm::vec3& val) { m_pPointLight->color = val; }

	private:
		bool OnPositionChanged(ObjectPositionChangedEvent& e);
		bool OnPropertyChanged(ObjectPropertyChangedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<PointLightComponent>(); }
		static std::string GetFactoryName() { return "LightComponent"; }

	private:
		PointLight* m_pPointLight;
	};
}