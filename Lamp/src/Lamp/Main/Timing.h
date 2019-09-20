#pragma once

namespace CactusEngine
{
	class FPSLimiter
	{
	public:
		FPSLimiter();

		//Funcitons
		void Initialize(float maxFPS);
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
