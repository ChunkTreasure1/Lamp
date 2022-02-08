#pragma once

#include "Lamp/Objects/Entity/EntityComponent.h"
#include "Lamp/Objects/Entity/ComponentRegistry.h"
#include "Lamp/Objects/Entity/Entity.h"

#include <Lamp/Objects/Entity/BaseComponents/Physics/RigidbodyComponent.h>

class ControllerComponent final : public Lamp::EntityComponent
{
public:
	ControllerComponent();

	/////Base/////
	void Initialize() override;
	void OnEvent(Lamp::Event& e) override;
	void SetComponentProperties() override;
	//////////////

	static Ref<EntityComponent> Create() { return CreateRef<ControllerComponent>(); }
	static std::string GetFactoryName() { return "ControllerComponent"; }

private:
	bool OnUpdateEvent(Lamp::AppUpdateEvent& e);

	float m_Speed = 10.f;
	Ref<Lamp::RigidbodyComponent> m_pRigidbody = nullptr;
};

