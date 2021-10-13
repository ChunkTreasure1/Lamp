#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Event/EntityEvent.h"

namespace Lamp
{
	class DirectionalLightComponent final : public EntityComponent
	{
	public:
		DirectionalLightComponent();
		~DirectionalLightComponent();

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		////////////////

		inline DirectionalLight* GetLight() { return m_pDirectionalLight; }

	private:
		bool OnRotationChanged(EntityRotationChangedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<DirectionalLightComponent>(); }
		static std::string GetFactoryName() { return "DirectionalLightComponent"; }

	private:
		DirectionalLight* m_pDirectionalLight;
	};
}