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
		inline const float GetIntensity() { return m_pPointLight->Intensity; }
		inline const float GetRadius() { return m_pPointLight->Radius; }
		inline const float GetFalloff() { return m_pPointLight->Falloff; }

		inline const glm::vec3& GetColor() { return m_pPointLight->Color; }
		inline const float GetFarPlane() { return m_pPointLight->FarPlane; }
		inline PointLight* GetLight() { return m_pPointLight; }

		//Setting
		inline void SetIntensity(float val) { m_pPointLight->Intensity = val; }
		inline void SetColor(const glm::vec3& val) { m_pPointLight->Color = val; }

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