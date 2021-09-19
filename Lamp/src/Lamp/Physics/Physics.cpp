#include "lppch.h"
#include "Physics.h"

#include "PhysXInternal.h"
#include "Lamp/Level/Level.h"

#include "PhysicsLayer.h"
#include "PhysXDebugger.h"

namespace Lamp
{
	PhysicsSettings Physics::s_Settings;
	Ref<PhysicsScene> Physics::s_Scene;

	void Physics::Initialize()
	{
		PhysXInternal::Initialize();
		PhysicsLayerManager::AddLayer("Default");
	}

	void Physics::Shutdown()
	{
		PhysXInternal::Shutdown();
	}

	void Physics::CreateScene()
	{
		s_Scene = CreateRef<PhysicsScene>(s_Settings);

#ifdef LP_DEBUG
		if (s_Settings.DebugOnPlay && !PhysXDebugger::IsDebugging())
		{
			PhysXDebugger::StartDebugging("PhysXDebugInfo", Physics::GetSettings().DebugType == DebugType::LiveDebug);
		}
#endif
	}

	void Physics::DestroyScene()
	{
#ifdef LP_DEBUG
		if (s_Settings.DebugOnPlay)
		{
			PhysXDebugger::StopDebugging();
		}
#endif
		s_Scene->Destroy();
	}

	void Physics::CreateActors(Level* pLevel)
	{
		for (auto& entity : pLevel->GetEntities())
		{
			if (entity.second->HasComponent<RigidbodyComponent>())
			{
				CreateActor(entity.second);
			}
		}
	}

	Ref<PhysicsActor> Physics::CreateActor(Entity* pEntity)
	{
		auto existingActor = s_Scene->GetActor(pEntity);
		if (existingActor)
		{
			return existingActor;
		}

		Ref<PhysicsActor> actor = s_Scene->CreateActor(pEntity);
		if (pEntity->HasComponent<BoxColliderComponent>())
		{
			actor->AddCollider(pEntity->GetComponent<BoxColliderComponent>(), pEntity, { 0.f, 0.f, 0.f });
		}

		if (pEntity->HasComponent<SphereColliderComponent>())
		{
			actor->AddCollider(pEntity->GetComponent<SphereColliderComponent>(), pEntity, { 0.f, 0.f, 0.f });
		}

		if (pEntity->HasComponent<CapsuleColliderComponent>())
		{
			actor->AddCollider(pEntity->GetComponent<CapsuleColliderComponent>(), pEntity, { 0.f, 0.f, 0.f });
		}

		actor->SetSimulationData(pEntity->GetComponent<RigidbodyComponent>()->GetSpecification().m_Layer);
		return actor;
	}
}