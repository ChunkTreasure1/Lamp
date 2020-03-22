#include "lppch.h"
#include "EnemyComponent.h"

#include <Lamp/Physics/Components/ConvexPolygonCollider.h>
#include <Lamp/Level/LevelSystem.h>
#include <Lamp/Physics/Components/RigidbodyComponent.h>

namespace Game
{
	void EnemyComponent::Initialize()
	{
		m_pEntity->SetScale({ 1.f / 1.5f, 1.f, 1.f });
		{
			mySpriteAnimationComponent = m_pEntity->GetOrCreateComponent<SpriteAnimationComponent>();
			mySpriteAnimationComponent->SetAnimationTime(200.f);

			mySpriteAnimationComponent->AddAnimation("Idle", Lamp::Texture2D::Create("engine/textures/Enemy/Idle/frame-1.png"));
			mySpriteAnimationComponent->AddAnimation("Idle", Lamp::Texture2D::Create("engine/textures/Enemy/Idle/frame-2.png"));

			mySpriteAnimationComponent->SetCurrentAnimSet("Idle");
		}

		{
			auto tempComp = m_pEntity->GetOrCreateComponent<Lamp::ConvexPolygonColliderComponent>();
			tempComp->SetTag("Enemy");
			tempComp->SetVerticeCount(4);

			tempComp->SetVerticePositions
			({
				{ -0.4f, -0.5f },
				{ -0.4f, 0.5f },
				{ 0.4f, 0.5f },
				{ 0.4f, -0.5f },
			});
		}
	}

	void EnemyComponent::Update(Lamp::Timestep someTS)
	{
		if (!myGrounded)
		{
			myVelocity -= glm::vec3(0.f, 0.01f, 0.f);
		}

		MoveTo(myCurrentWantedPos);
		m_pEntity->SetPosition(m_pEntity->GetPosition() + myVelocity * (float)someTS);
	}

	void EnemyComponent::OnEvent(Lamp::Event& someE)
	{
		Lamp::EventDispatcher tempDispatcher(someE);
		tempDispatcher.Dispatch<Lamp::EntityCollisionEvent>(LP_BIND_EVENT_FN(EnemyComponent::OnCollided));
	}

	void EnemyComponent::Draw()
	{
	}

	bool EnemyComponent::OnCollided(Lamp::EntityCollisionEvent& someEvent)
	{
		if (someEvent.GetTag() == "Bullet")
		{
			Lamp::LevelSystem::GetCurrentLevel()->GetEntityManager().Remove(GetOwner());
		}
		else if (someEvent.GetTag() == "Ground")
		{
			myVelocity.y = 0.f;
			myGrounded = true;
		}

		return false;
	}

	void EnemyComponent::MoveTo(const glm::vec3& somePosition)
	{
		if (m_pEntity->GetPosition() != somePosition)
		{
			glm::vec3 tempDir = m_pEntity->GetPosition() - somePosition;
			tempDir = glm::normalize(tempDir);

			myVelocity.x += tempDir.x * 3.f;
		}
	}
}