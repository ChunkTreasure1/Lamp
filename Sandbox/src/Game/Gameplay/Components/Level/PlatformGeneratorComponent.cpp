#include "lppch.h"
#include "PlatformGeneratorComponent.h"

#include <Lamp/Level/LevelSystem.h>
#include "Game/Gameplay/Components/Enemies/EnemyComponent.h"

namespace Game
{
	bool PlatformGeneratorComponent::s_Registered = LP_REGISTER_COMPONENT(PlatformGeneratorComponent)

	void PlatformGeneratorComponent::Initialize()
	{
	}

	void PlatformGeneratorComponent::Update(Lamp::Timestep someTS)
	{
		for (size_t i = 0; i < myPlatforms.size(); i++)
		{
			if (glm::distance(myPlatforms[i]->GetPosition(), myPlayer->GetPosition()) > 10)
			{
				Lamp::LevelSystem::GetCurrentLevel()->GetEntityManager().Remove(myPlatforms[i]);

				i--;
			}
		}

		if (myPlatforms.size() < 6)
		{
			for (size_t i = 0; i < 6 - myPlatforms.size(); i++)
			{
				auto tempEnt = Lamp::LevelSystem::GetCurrentLevel()->GetEntityManager().Create();
				tempEnt->GetOrCreateComponent<PlatformComponent>();
				tempEnt->SetPosition({ 0.f, myPlayer->GetPosition().y + 2 * i + 2, 0.f });

				auto tempEn = Lamp::LevelSystem::GetCurrentLevel()->GetEntityManager().Create();
				tempEn->SetPosition({ tempEnt->GetPosition() + glm::vec3(0.f, 1.3f, 0.f) });
				tempEn->GetOrCreateComponent<EnemyComponent>()->SetWantedPos(tempEn->GetPosition() + glm::vec3(4.f, 0.f, 0.f));

				myPlatforms.push_back(tempEnt);
			}
		}
	}

	void PlatformGeneratorComponent::OnEvent(Lamp::Event& someE)
	{
	}
}