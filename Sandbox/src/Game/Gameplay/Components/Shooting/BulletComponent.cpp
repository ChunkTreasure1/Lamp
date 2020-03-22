#include "lppch.h"
#include "BulletComponent.h"

#include <Lamp/Entity/Base/ComponentRegistry.h>
#include <Lamp/Entity/Base/Entity.h>
#include <Lamp/Rendering/Renderer2D.h>

#include <Lamp/Physics/Components/ConvexPolygonCollider.h>

namespace Game
{
	bool BulletComponent::s_Registered = LP_REGISTER_COMPONENT(BulletComponent);

	void BulletComponent::Initialize()
	{
		myTexture = Lamp::Texture2D::Create("engine/textures/ff.png");
		m_pEntity->SetScale({ 0.3f, 0.3f, 0.3f });
	
		{
			auto tempComp = m_pEntity->GetOrCreateComponent<Lamp::ConvexPolygonColliderComponent>();
			tempComp->SetVerticeCount(4);
			tempComp->SetTag("Bullet");

			tempComp->SetVerticePositions
			({
				{ -0.15f, -0.15f },
				{ -0.15f, 0.15f },
				{ 0.15f, 0.15f },
				{ 0.15f, -0.15 },
			});

		}
	}

	void BulletComponent::Update(Lamp::Timestep someTS)
	{
		m_pEntity->SetPosition(m_pEntity->GetPosition() + myDirection * 20.f * (float)someTS);
	}

	void BulletComponent::OnEvent(Lamp::Event& e)
	{
	}

	void BulletComponent::Draw()
	{
		Lamp::Renderer2D::DrawQuad(m_pEntity->GetPosition(), m_pEntity->GetScale(), m_pEntity->GetRotation().x, myTexture);
	}
}