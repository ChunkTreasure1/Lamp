#pragma once

#include <PhysX/PxPhysicsAPI.h>

#include <string>

namespace Lamp
{
	class PhysXDebugger
	{
	public:
		static void Initialize();
		static void Shutdown();
		
		static void StartDebugging(const std::string& filepath, bool networkDebug = false);
		static bool IsDebugging();
		static void StopDebugging();

		static physx::PxPvd* GetDebugger();
	};
}