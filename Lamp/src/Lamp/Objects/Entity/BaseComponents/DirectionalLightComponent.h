#pragma once

#include "Lamp/Objects/Entity/EntityComponent.h"
#include "Lamp/Event/EntityEvent.h"

namespace Lamp
{
	class DirectionalLightComponent final : public EntityComponent
	{
	public:
		DirectionalLightComponent();
		~DirectionalLightComponent() override;

		//////Base//////
		void Initialize() override;
		void OnEvent(Event& e) override;
		void SetComponentProperties() override;
		////////////////

		inline DirectionalLight& GetLight() { return *m_pDirectionalLight; }

		static Ref<EntityComponent> Create() { return CreateRef<DirectionalLightComponent>(); }
		static std::string GetFactoryName() { return "DirectionalLightComponent"; }

	private:
		friend class LevelLoader;

		bool OnRotationChanged(ObjectRotationChangedEvent& e);
		void UpdateLight();

		Scope<DirectionalLight> m_pDirectionalLight;
		const float m_size = 20.f;
	};
}