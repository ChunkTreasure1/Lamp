#pragma once

#include <Lamp/Entity/Base/BaseComponent.h>
#include <Lamp/Entity/Base/ComponentRegistry.h>
#include <Lamp/Entity/Base/Entity.h>

#include <Lamp.h>

namespace Game
{
	class PlatformComponent : public Lamp::IEntityComponent
	{
	public:

		PlatformComponent() 
			: IEntityComponent("PlatformComponent")
		{}
		~PlatformComponent() {}

		//////Base//////
		virtual void Initialize() override;
		virtual void Update(Lamp::Timestep someTS) override;
		virtual void OnEvent(Lamp::Event& someE) override;
		virtual void Draw() override;
		virtual void SetProperty(Lamp::ComponentProperty& someProp, void* someData) override {}
		////////////////

	public:
		static Ref<IEntityComponent> Create() { return std::make_shared<PlatformComponent>(); }
		static std::string GetFactoryName() { return "PlatformComponent"; }

	private:
		static bool s_Registered;

	private:
		std::vector<Ref<Lamp::Texture2D>> myTextures;

		float myWidth;
		float myHeight;
	};
}