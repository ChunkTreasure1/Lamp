#pragma once

#include "Collision.h"

namespace Lamp
{
	class Physics
	{
	public:

		static void CheckCollisions() { Collision::CheckCollisions(); }

		//Getting
		static const float GetGravity() { return m_Gravity; }

		//Setting
		static inline void SetGravity(float val) { m_Gravity = val; }

	private:
		Physics() = delete;

		static float m_Gravity;
	};

}