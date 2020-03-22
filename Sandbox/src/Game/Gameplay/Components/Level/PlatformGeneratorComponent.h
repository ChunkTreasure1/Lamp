#pragma once

#include <Lamp/Entity/Base/BaseComponent.h>
#include <Lamp/Entity/Base/ComponentRegistry.h>
#include <Lamp/Entity/Base/Entity.h>

#include "PlatformComponent.h"

namespace Game
{
	class PlatformGeneratorComponent : public Lamp::IEntityComponent
	{
	public:

		PlatformGeneratorComponent() 
			: IEntityComponent("PlatformGeneratorComponent")
		{}
		~PlatformGeneratorComponent() {}

		//////Base//////
		virtual void Initialize() override;
		virtual void Update(Lamp::Timestep someTS) override;
		virtual void OnEvent(Lamp::Event& someE) override;
		virtual void Draw() override {}
		virtual void SetProperty(Lamp::ComponentProperty& someProp, void* someData) override {}
		////////////////

		//Setting
		inline void SetPlayer(Lamp::IEntity* somePlayer) { myPlayer = somePlayer; }

	public:
		static Ref<IEntityComponent> Create() { return std::make_shared<PlatformGeneratorComponent>(); }
		static std::string GetFactoryName() { return "PlatformGeneratorComponent"; }

	private:
		static bool s_Registered;

	private:
		std::vector<Lamp::IEntity*> myPlatforms;
		Lamp::IEntity* myPlayer;
	};
}