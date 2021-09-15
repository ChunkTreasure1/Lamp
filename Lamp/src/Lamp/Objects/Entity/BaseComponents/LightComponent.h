#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/Meshes/Mesh.h"
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
		virtual uint32_t GetSize() { return sizeof(*this); }
		virtual uint64_t GetEventMask() override { return EventType::EntityPositionChanged; }
		////////////////

		//Getting
		inline const float GetIntensity() { return m_pPointLight->Intensity; }
		inline const float GetRadius() { return m_pPointLight->Radius; }
		inline const float GetFalloff() { return m_pPointLight->Falloff; }

		inline const glm::vec3& GetColor() { return m_pPointLight->Color; }
		inline const float GetFarPlane() { return m_pPointLight->FarPlane; }
		inline PointLight* GetPointLight() { return m_pPointLight; }

		//Setting
		inline void SetIntensity(float val) { m_pPointLight->Intensity = val; }
		inline void SetColor(const glm::vec3& val) { m_pPointLight->Color = val; }

	private:
		bool OnPositionChanged(EntityPositionChangedEvent& e);
		bool OnPropertyChanged(EntityPropertyChangedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<LightComponent>(); }
		static std::string GetFactoryName() { return "LightComponent"; }

	private:
		PointLight* m_pPointLight;
	};
}