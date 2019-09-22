#pragma once

namespace Lamp
{
	class FPSLimiter
	{
	public:
		FPSLimiter(float maxFPS);

		//Funcitons
		void Begin();
		float End();

		void SetMaxFPS(float maxFPS) { m_MaxFPS = maxFPS; }

	private:

		void CalculateFPS();

		//Member vars
		float m_MaxFPS;
		float m_FPS;
		float m_FrameTime;

		unsigned int m_StartTicks;
	};
}
