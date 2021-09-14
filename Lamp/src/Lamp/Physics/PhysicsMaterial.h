#pragma once

namespace Lamp
{
	class PhysicsMaterial
	{
	public:
		PhysicsMaterial() = default;
		PhysicsMaterial(float staticFriction, float dynamicFriction, float bounciness)
			: StaticFriction(staticFriction), DynamicFriction(dynamicFriction), Bounciness(bounciness)
		{}

	public:
		float StaticFriction = 0.f;
		float DynamicFriction = 0.f;
		float Bounciness = 0.f;
	};
}