#include "lppch.h"
#include "PlayerComponent.h"

#include <Lamp/Entity/Base/ComponentRegistry.h>
#include <Lamp/Input/KeyCodes.h>
#include <Lamp/Level/LevelSystem.h>

#include "Game/Gameplay/Components/General/SpriteAnimationComponent.h"
#include "Game/Gameplay/Components/Shooting/BulletComponent.h"
#include <Lamp/Physics/Components/ConvexPolygonCollider.h>
#include <Lamp/Physics/Components/RigidbodyComponent.h>

namespace Game
{
	bool PlayerComponent::s_Registered = LP_REGISTER_COMPONENT(PlayerComponent);

	void PlayerComponent::Initialize()
	{
		m_pEntity->SetScale({ 1.f / 1.5f, 1.f, 1.f });
		myCameraController = new Lamp::OrthographicCameraController(Lamp::Application::Get().GetWindow().GetWidth() / Lamp::Application::Get().GetWindow().GetHeight());

		{
			mySpriteAnimationComponent = m_pEntity->GetOrCreateComponent<SpriteAnimationComponent>();
			mySpriteAnimationComponent->SetAnimationTime(200.f);

			mySpriteAnimationComponent->AddAnimation("Idle", Lamp::Texture2D::Create("engine/textures/Player/Idle/frame-1.png"));
			mySpriteAnimationComponent->AddAnimation("Idle", Lamp::Texture2D::Create("engine/textures/Player/Idle/frame-2.png"));

			mySpriteAnimationComponent->AddAnimation("JumpUpRight", Lamp::Texture2D::Create("engine/textures/Player/Jump/JumpUpRight.png"));
			mySpriteAnimationComponent->AddAnimation("JumpDownRight", Lamp::Texture2D::Create("engine/textures/Player/Jump/JumpFallRight.png"));

			mySpriteAnimationComponent->AddAnimation("JumpUpLeft", Lamp::Texture2D::Create("engine/textures/Player/Jump/JumpUpLeft.png"));
			mySpriteAnimationComponent->AddAnimation("JumpDownLeft", Lamp::Texture2D::Create("engine/textures/Player/Jump/JumpFallLeft.png"));

			mySpriteAnimationComponent->AddAnimation("RunRight", Lamp::Texture2D::Create("engine/textures/Player/RunRight/frame-1.png"));
			mySpriteAnimationComponent->AddAnimation("RunRight", Lamp::Texture2D::Create("engine/textures/Player/RunRight/frame-2.png"));
			mySpriteAnimationComponent->AddAnimation("RunRight", Lamp::Texture2D::Create("engine/textures/Player/RunRight/frame-3.png"));
			mySpriteAnimationComponent->AddAnimation("RunRight", Lamp::Texture2D::Create("engine/textures/Player/RunRight/frame-4.png"));
			mySpriteAnimationComponent->AddAnimation("RunRight", Lamp::Texture2D::Create("engine/textures/Player/RunRight/frame5.png"));
			mySpriteAnimationComponent->AddAnimation("RunRight", Lamp::Texture2D::Create("engine/textures/Player/RunRight/frame-6.png"));

			mySpriteAnimationComponent->AddAnimation("RunLeft", Lamp::Texture2D::Create("engine/textures/Player/RunLeft/frame-1.png"));
			mySpriteAnimationComponent->AddAnimation("RunLeft", Lamp::Texture2D::Create("engine/textures/Player/RunLeft/frame-2.png"));
			mySpriteAnimationComponent->AddAnimation("RunLeft", Lamp::Texture2D::Create("engine/textures/Player/RunLeft/frame-3.png"));
			mySpriteAnimationComponent->AddAnimation("RunLeft", Lamp::Texture2D::Create("engine/textures/Player/RunLeft/frame-4.png"));
			mySpriteAnimationComponent->AddAnimation("RunLeft", Lamp::Texture2D::Create("engine/textures/Player/RunLeft/frame-5.png"));
			mySpriteAnimationComponent->AddAnimation("RunLeft", Lamp::Texture2D::Create("engine/textures/Player/RunLeft/frame-6.png"));

			mySpriteAnimationComponent->SetCurrentAnimSet("Idle");
		}

		{
			auto tempComp = m_pEntity->GetOrCreateComponent<Lamp::ConvexPolygonColliderComponent>();
			tempComp->SetVerticeCount(4);
			tempComp->SetVerticePositions(
				{
					{ -0.4f, -0.5f },
					{ -0.4f, 0.5f },
					{ 0.4f, 0.5f },
					{ 0.4f, -0.5f },
				});
		}
	}

	void PlayerComponent::Update(Lamp::Timestep someTS)
	{
		m_pEntity->SetPosition(m_pEntity->GetPosition() + myVelocity * (float)someTS);
		myCameraController->SetPosition(m_pEntity->GetPosition());

		if (!myGrounded)
		{
			myVelocity -= glm::vec3(0.f, 0.1f, 0.f);

			if (myVelocity.y > 0)
			{
				if (myVelocity.x > 0)
				{
					mySpriteAnimationComponent->SetCurrentAnimSet("JumpUpRight");
				}
				else 
				{
					mySpriteAnimationComponent->SetCurrentAnimSet("JumpUpLeft");
				}
			}
			else if (myVelocity.y < 0 )
			{
				if (myVelocity.x > 0)
				{
					mySpriteAnimationComponent->SetCurrentAnimSet("JumpDownRight");
				}
				else
				{
					mySpriteAnimationComponent->SetCurrentAnimSet("JumpDownLeft");
				}
			}
		}

		CheckForInput(someTS);
		myCameraController->Update(someTS);

		for (size_t i = 0; i < myBullets.size(); i++)
		{
			if (glm::distance(m_pEntity->GetPosition(), myBullets[i]->GetPosition()) > 20.f)
			{
				myBullets[i]->Destroy();

				Lamp::LevelSystem::GetCurrentLevel()->GetEntityManager().Remove(myBullets[i]);

				if (auto tempIt = std::find(myBullets.begin(), myBullets.end(), myBullets[i]); tempIt != myBullets.end())
				{
					myBullets.erase(tempIt);
				}

				i--;
			}
		}
	}

	void PlayerComponent::OnEvent(Lamp::Event& someEvent)
	{
		myCameraController->OnEvent(someEvent);

		Lamp::EventDispatcher tempDispatcher(someEvent);
		tempDispatcher.Dispatch<Lamp::MouseMovedEvent>(LP_BIND_EVENT_FN(PlayerComponent::OnMouseMoved));
		tempDispatcher.Dispatch<Lamp::EntityCollisionEvent>(LP_BIND_EVENT_FN(PlayerComponent::OnPlayerCollided));
		tempDispatcher.Dispatch<Lamp::EntityStoppedCollisionEvent>(LP_BIND_EVENT_FN(PlayerComponent::OnPlayerStopColliding));

		tempDispatcher.Dispatch<Lamp::KeyPressedEvent>(LP_BIND_EVENT_FN(PlayerComponent::OnKeyPressed));
		tempDispatcher.Dispatch<Lamp::KeyReleasedEvent>(LP_BIND_EVENT_FN(PlayerComponent::OnKeyReleased));
	}

	void PlayerComponent::CheckForInput(Lamp::Timestep someTS)
	{
		if (Lamp::Input::IsMouseButtonPressed(0))
		{
			Shoot();
		}

	}

	void PlayerComponent::Shoot()
	{
		auto tempEnt = Lamp::LevelSystem::GetCurrentLevel()->GetEntityManager().Create();
		auto tempComp = tempEnt->GetOrCreateComponent<BulletComponent>();

		tempEnt->SetPosition(m_pEntity->GetPosition());
		tempComp->SetDirection(glm::normalize(myCurrentMousePos - m_pEntity->GetPosition()));

		myBullets.push_back(tempEnt);
	}

	bool PlayerComponent::OnMouseMoved(Lamp::MouseMovedEvent& someEvent)
	{
		uint32_t tempHeight = Lamp::Application::Get().GetWindow().GetHeight();
		uint32_t tempWidth = Lamp::Application::Get().GetWindow().GetWidth();

		glm::vec2 tempPos = myCameraController->ScreenToWorldCoords(glm::vec2(someEvent.GetX(), someEvent.GetY()), { Lamp::Application::Get().GetWindow().GetWidth(), Lamp::Application::Get().GetWindow().GetHeight() });
		myCurrentMousePos = { tempPos.x, tempPos.y, 0.f };

		return false;
	}

	bool PlayerComponent::OnPlayerCollided(Lamp::EntityCollisionEvent& someEvent)
	{
		if (someEvent.GetTag() == "Ground")
		{
			myGrounded = true;
			myJumpAmount = 0;
			myVelocity.y = 0.f;

			if (myVelocity.x < 0)
			{
				mySpriteAnimationComponent->SetCurrentAnimSet("RunLeft");
			}
			else if (myVelocity.x > 0)
			{
				mySpriteAnimationComponent->SetCurrentAnimSet("RunRight");
			}
			else 
			{
				mySpriteAnimationComponent->SetCurrentAnimSet("Idle");
			}
		}

		return false;
	}

	bool PlayerComponent::OnPlayerStopColliding(Lamp::EntityStoppedCollisionEvent& someEvent)
	{
		if (someEvent.GetTag() == "Ground")
		{
			myGrounded = false;
		}

		return true;
	}

	bool PlayerComponent::OnKeyPressed(Lamp::KeyPressedEvent& someEvent)
	{
		if (someEvent.GetKeyCode() == LP_KEY_SPACE && (myGrounded || myJumpAmount < 2))
		{
			myJumpAmount++;
			myJumped = true;
			myGrounded = false;

			myVelocity += glm::vec3(0.f, 5.f, 0.f);
		}
		if (someEvent.GetKeyCode() == LP_KEY_A)
		{
			myVelocity.x = -5.f;
			mySpriteAnimationComponent->SetCurrentAnimSet("RunLeft");
		}
		else if (someEvent.GetKeyCode() == LP_KEY_D)
		{
			myVelocity.x = 5.f;
			mySpriteAnimationComponent->SetCurrentAnimSet("RunRight");
		}

		return true;
	}

	bool PlayerComponent::OnKeyReleased(Lamp::KeyReleasedEvent& someEvent)
	{
		if (someEvent.GetKeyCode() == LP_KEY_D|| someEvent.GetKeyCode() == LP_KEY_A)
		{
			myVelocity.x = 0.f;
			mySpriteAnimationComponent->SetCurrentAnimSet("Idle");
		}

		return true;
	}
}