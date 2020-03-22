#pragma once

#include <Lamp/Entity/Base/BaseComponent.h>
#include <Lamp/Entity/Base/ComponentRegistry.h>
#include <Lamp/Entity/Base/Entity.h>

#include <Lamp.h>

namespace Lamp
{
	class RigidbodyComponent : public Lamp::IEntityComponent
	{
	public:

		RigidbodyComponent()
			: IEntityComponent("RigidbodyComponent")
		{}
		~RigidbodyComponent() {}

		//////Base//////
		virtual void Initialize() override;
		virtual void Update(Lamp::Timestep someTS) override;
		virtual void OnEvent(Lamp::Event& someE) override;
		virtual void Draw() override;
		virtual void SetProperty(Lamp::ComponentProperty& someProp, void* someData) override {}
		////////////////

	private:

	public:
		static Ref<IEntityComponent> Create() { return std::make_shared<RigidbodyComponent>(); }
		static std::string GetFactoryName() { return "RigidbodyComponent"; }

	private:
		static bool s_Registered;

	private:
	};
}