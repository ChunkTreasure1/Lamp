#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include <btBulletDynamicsCommon.h>

namespace Lamp
{
	class CapsuleColliderComponent final : public EntityComponent
	{
	public:
		CapsuleColliderComponent();

		//////Base//////
		virtual void Initialize() override;
		virtual void OnEvent(Event& e) override;
		virtual uint64_t GetEventMask() override
		{
			return EventType::AppUpdate
				| EventType::EntityPropertyChanged
				| EventType::EntityComponentAdded;
		}
		////////////////

	private:
		bool OnPropertyChanged(EntityPropertyChangedEvent& e);
		bool OnComponentAdded(EntityComponentAddedEvent& e);

	public:
		static Ref<EntityComponent> Create() { return CreateRef<CapsuleColliderComponent>(); }
		static std::string GetFactoryName() { return "CapsuleColliderComponent"; }

	private:
		btCapsuleShape* m_pShape = nullptr;

		bool m_RigidbodyFound = false;
		float m_Radius = 0.5f;
		float m_Height = 1.f;
	};
}