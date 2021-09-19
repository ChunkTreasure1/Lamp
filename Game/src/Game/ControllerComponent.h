#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

#include <Lamp/Objects/Entity/BaseComponents/Physics/RigidbodyComponent.h>

class ControllerComponent final : public Lamp::EntityComponent
{
public:
	ControllerComponent()
		: EntityComponent("ControllerComponent")
	{
		SetComponentProperties
		({
			{ Lamp::PropertyType::Float, "Speed", RegisterData(&m_Speed) }
		});
	}

	/////Base/////
	virtual void Initialize() override;
	virtual void OnEvent(Lamp::Event& e) override;
	virtual uint32_t GetSize() { return sizeof(*this); }
	virtual uint64_t GetEventMask() override { return Lamp::EventType::All; }
	//////////////

public:
	static Ref<EntityComponent> Create() { return CreateRef<ControllerComponent>(); }
	static std::string GetFactoryName() { return "ControllerComponent"; }

private:
	bool OnUpdateEvent(Lamp::AppUpdateEvent& e);

private:
	float m_Speed = 10.f;
	Ref<Lamp::RigidbodyComponent> m_pRigidbody = nullptr;
};

