#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include "Lamp/Event/ApplicationEvent.h"

namespace Lamp
{
	class Rigidbody;

	class RigidbodyComponent final : public EntityComponent
	{
	public:
		RigidbodyComponent();

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint64_t GetEventMask() override 
		{ 
			return EventType::AppUpdate
				| EventType::EntityPropertyChanged; 
		}
		////////////////

		inline Rigidbody* GetRigidbody() { return m_pRigidbody; }

	private:
		bool OnUpdate(AppUpdateEvent& e);
		bool OnEntityPropertyChanged(EntityPropertyChangedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<RigidbodyComponent>(); }
		static std::string GetFactoryName() { return "RigidbodyComponent"; }

	private:
		Rigidbody* m_pRigidbody = nullptr;
		bool m_IsStatic = false;
		float m_Mass = 1.f;
	};
}