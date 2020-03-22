#pragma once

#include <Lamp.h>
#include "Game/Gameplay/Components/General/SpriteAnimationComponent.h"
#include <Lamp/Event/EntityEvent.h>

namespace Game
{
	class PlayerComponent : public Lamp::IEntityComponent
	{
	public:
		PlayerComponent() 
			: IEntityComponent("PlayerComponent"), myVelocity({0.f, 0.f, 0.f})
		{}
		~PlayerComponent() 
		{
			delete myCameraController;
		}

		//////Base//////
		virtual void Initialize() override;
		virtual void Update(Lamp::Timestep someTS) override;
		virtual void OnEvent(Lamp::Event& someEvent) override;
		virtual void Draw() override {}
		virtual void SetProperty(Lamp::ComponentProperty& prop, void* pData) {}
		////////////////

		inline Lamp::OrthographicCameraController* GetCamera() { return myCameraController; }

	private:
		void CheckForInput(Lamp::Timestep someTS);
		void Shoot();
		bool OnMouseMoved(Lamp::MouseMovedEvent& someEvent);
		bool OnPlayerCollided(Lamp::EntityCollisionEvent& someEvent);
		bool OnPlayerStopColliding(Lamp::EntityStoppedCollisionEvent& someEvent);

		bool OnKeyPressed(Lamp::KeyPressedEvent& someEvent);
		bool OnKeyReleased(Lamp::KeyReleasedEvent& someEvent);

	private:
		Lamp::OrthographicCameraController* myCameraController;
		Ref<SpriteAnimationComponent> mySpriteAnimationComponent;

		glm::vec3 myCurrentMousePos;
		glm::vec3 myVelocity;
		std::vector<Lamp::IEntity*> myBullets;

		bool myJumped = false;
		bool myGrounded = false;
		float myMaxSpeed = 5.f;

		uint32_t myJumpAmount = 0;

	public:
		static Ref<IEntityComponent> Create() { return std::make_shared<PlayerComponent>(); }
		static std::string GetFactoryName() { return "PlayerComponent"; }

	private:
		static bool s_Registered;
	};
}