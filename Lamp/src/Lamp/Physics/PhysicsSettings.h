#pragma once

#include <glm/glm.hpp>

namespace Lamp
{ 
	enum class BroadphaseType
	{
		SweepAndPrune,
		MutliBoxPrune,
		AutomaticBoxPrune
	};

	enum class FrictionType
	{
		Patch,
		OneDirectional,
		TwoDirectional
	};

	enum class DebugType
	{
		DebugToFile = 0,
		LiveDebug
	};

	struct PhysicsSettings
	{
		float FixedTimestep = 1.f / 100.f;
		glm::vec3 Gravity = { 0.f, -9.81f, 0.f };
		
		BroadphaseType BroadphaseAlgorithm = BroadphaseType::AutomaticBoxPrune;
		FrictionType FrictionModel = FrictionType::Patch;

		glm::vec3 WorldBoundsMin = glm::vec3(-100.f);
		glm::vec3 WorldBoundsMax = glm::vec3(100.f);
		uint32_t WorldBoundsSubdivisions = 2;

		uint32_t SolverIterations = 8;
		uint32_t SolverVelocityIterations = 2;

#ifdef LP_DEBUG
		bool DebugOnPlay = true;
		DebugType DebugType = DebugType::DebugToFile;
#endif
	};
}