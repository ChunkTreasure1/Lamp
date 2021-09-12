#pragma once

#include "Lamp/Objects/Entity/Base/BaseComponent.h"
#include "Lamp/Objects/Entity/Base/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Base/Entity.h"

class ControllerComponent final : public Lamp::EntityComponent
{
public:
	ControllerComponent()
		: EntityComponent("ControllerComponent")
	{
	}

	/////Base/////
	virtual void Initialize() override;
	virtual void OnEvent(Lamp::Event& e) override;
	virtual uint64_t GetEventMask() override { return Lamp::EventType::All; }
	//////////////

public:
	static Ref<EntityComponent> Create() { return CreateRef<ControllerComponent>(); }
	static std::string GetFactoryName() { return "ControllerComponent"; }

private:
	bool OnUpdateEvent(Lamp::AppUpdateEvent& e);

private:
};

