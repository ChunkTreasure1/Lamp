#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include <btBulletDynamicsCommon.h>

namespace Lamp
{
	class SphereColliderComponent final : public EntityComponent
	{
	public:
		SphereColliderComponent();

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint64_t GetEventMask() override
		{
			return EventType::AppUpdate
				| EventType::EntityPropertyChanged
				| EventType::EntityComponentAdded;;
		}
		////////////////

	private:
		bool OnPropertyChanged(EntityPropertyChangedEvent& e);
		bool OnComponentAdded(EntityComponentAddedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<SphereColliderComponent>(); }
		static std::string GetFactoryName() { return "SphereColliderComponent"; }

	private:
		btSphereShape* m_pShape;

		float m_Radius = 1.f;
		bool m_RigidbodyFound = false;
	};
}