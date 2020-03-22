#include "lppch.h"
#include "PlatformComponent.h"

#include <Lamp/Physics/Components/ConvexPolygonCollider.h>
#include <Lamp/Rendering/Texture2D/Texture2D.h>
#include <Lamp/Utility/Random.h>

#include <Lamp/Level/LevelSystem.h>

namespace Game
{
	bool PlatformComponent::s_Registered = LP_REGISTER_COMPONENT(PlatformComponent)

	void PlatformComponent::Initialize()
	{
		myWidth = Lamp::Random::Int(4, 8);
		myHeight = 0.5;

		m_pEntity->SetScale({ 1.f, 0.5f, 1.f });

		{
			auto tempComp = m_pEntity->GetOrCreateComponent<Lamp::ConvexPolygonColliderComponent>();
			tempComp->SetVerticeCount(4);
			tempComp->SetTag("Ground");
			tempComp->SetMass(1.f);

			tempComp->SetVerticePositions
			({
				{ -myWidth / 2.f, -myHeight / 2.f },
				{ -myWidth / 2.f, myHeight / 2 },
				{ myWidth / 2, myHeight / 2 },
				{ myWidth / 2, -myHeight / 2 },
			});

		}

		for (size_t i = 0; i < myWidth; i++)
		{
			myTextures.push_back(Lamp::Texture2D::Create("engine/textures/GroundTiles/tile000.png"));
		}
	}

	void PlatformComponent::Update(Lamp::Timestep someTS)
	{
	}

	void PlatformComponent::OnEvent(Lamp::Event& someE)
	{
	}

	void PlatformComponent::Draw()
	{
		for(auto& tempT : myTextures)
		{
			for (size_t i = 0; i < myWidth / 2; i++)
			{
				Lamp::Renderer2D::DrawQuad(m_pEntity->GetPosition() + glm::vec3(i + 0.5f, 0, 0), m_pEntity->GetScale(), tempT);
				Lamp::Renderer2D::DrawQuad(m_pEntity->GetPosition() - glm::vec3(i + 0.5f, 0, 0), m_pEntity->GetScale(), tempT);
			}
		}
	}
}