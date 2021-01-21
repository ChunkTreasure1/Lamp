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

		LightComponent();
		~LightComponent();

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint64_t GetEventMask() override { return EventType::EntityPositionChanged; }
		////////////////

		//Getting
		inline const float GetIntensity() { return m_PointLight.Intensity; }
		inline const float GetRadius() { return m_PointLight.Radius; }
		inline const float GetFalloff() { return m_PointLight.Falloff; }

		inline const glm::vec3& GetColor() { return m_PointLight.Color; }

		//Setting
		inline void SetIntensity(float val) { m_PointLight.Intensity = val; }
		inline void SetColor(const glm::vec3& val) { m_PointLight.Color = val; }

	private:
		bool OnRender(AppRenderEvent& e);
		bool OnUpdate(AppUpdateEvent& e);
		bool OnPositionChanged(EntityPositionChangedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<LightComponent>(); }
		static std::string GetFactoryName() { return "LightComponent"; }

	private:
		PointLight m_PointLight;
	};
}