#pragma once

#include <Lamp/Entity/Base/BaseComponent.h>
#include <Lamp/Entity/Base/ComponentRegistry.h>
#include <Lamp/Entity/Base/Entity.h>

#include <Lamp.h>
#include "Game/Gameplay/Components/General/SpriteAnimationComponent.h"
#include <Lamp/Event/EntityEvent.h>

namespace Game
{
	class EnemyComponent : public Lamp::IEntityComponent
	{
	public:

		EnemyComponent()
			: IEntityComponent("EnemyComponent"), myVelocity(0.f), myCurrentWantedPos(0.f)
		{}
		~EnemyComponent() {}

		//////Base//////
		virtual void Initialize() override;
		virtual void Update(Lamp::Timestep someTS) override;
		virtual void OnEvent(Lamp::Event& someE) override;
		virtual void Draw() override;
		virtual void SetProperty(Lamp::ComponentProperty& someProp, void* someData) override {}
		////////////////

		void SetWantedPos(const glm::vec3& somePos) { myCurrentWantedPos = somePos; }

	private:
		bool OnCollided(Lamp::EntityCollisionEvent& someEvent);
		void MoveTo(const glm::vec3& somePosition);

	public:
		static Ref<IEntityComponent> Create() { return std::make_shared<EnemyComponent>(); }
		static std::string GetFactoryName() { return "EnemyComponent"; }

	private:
		static bool s_Registered;

	private:
		Ref<SpriteAnimationComponent> mySpriteAnimationComponent;
		glm::vec3 myVelocity;
		glm::vec3 myCurrentWantedPos;
		bool myGrounded = false;
	};
}