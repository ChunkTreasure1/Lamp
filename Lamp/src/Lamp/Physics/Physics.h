#pragma once

#include "PhysicsScene.h"

namespace Lamp
{
	class Physics
	{
	public:
		static void Initialize();
		static void Shutdown();

		static Ref<PhysicsScene> GetScene() { return s_Scene; }

		static void CreateScene();
		static void DestroyScene();

		static void CreateActors(Level* pLevel);
		static Ref<PhysicsActor> CreateActor(Entity* pEntity);

		static PhysicsSettings& GetSettings() { return s_Settings; }

	private:
		static PhysicsSettings s_Settings;
		static Ref<PhysicsScene> s_Scene;
	};
}