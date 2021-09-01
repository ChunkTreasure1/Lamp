#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include <btBulletDynamicsCommon.h>

namespace Lamp
{
	class BoxColliderComponent final : public EntityComponent
	{
	public:
		BoxColliderComponent();

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
		static Ref<EntityComponent> Create() { return CreateRef<BoxColliderComponent>(); }
		static std::string GetFactoryName() { return "BoxColliderComponent"; }

	private:
		btBoxShape* m_pShape = nullptr;
		glm::vec3 m_Scale = { 1.f, 1.f, 1.f };
		bool m_RigidbodyFound = false;
	};
}