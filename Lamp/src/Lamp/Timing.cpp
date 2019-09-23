#include "lppch.h"
#include "Timing.h"
#include "GLFW/glfw3.h"

namespace Lamp
{
	FPSLimiter::FPSLimiter(float maxFPS)
	{
		SetMaxFPS(maxFPS);
	}

	//Called on the beginning of a frame
	void FPSLimiter::Begin()
	{
		m_StartTicks = glfwGetTime();
	}

	//Called on the end of a frame
	float FPSLimiter::End()
	{
		CalculateFPS();

		return m_FPS;
	}

	//Calculates the FPS
	void FPSLimiter::CalculateFPS()
	{
		static const int NUM_SAMPLES = 100;
		static float frameTimes[NUM_SAMPLES];
		static int currentFrame = 0;

		static uint32_t prevTicks = glfwGetTime();

		uint32_t currentTicks;
		currentTicks = glfwGetTime();

		m_FrameTime = currentTicks - prevTicks;
		frameTimes[currentFrame % NUM_SAMPLES] = m_FrameTime;

		prevTicks = currentTicks;
		int count;

		currentFrame++;
		if (currentFrame < NUM_SAMPLES)
		{
			count = currentFrame;
		}
		else
		{
			count = NUM_SAMPLES;
		}

		float frameTimeAverage = 0;
		for (int i = 0; i < count; i++)
		{
			frameTimeAverage += frameTimes[i];
		}

		frameTimeAverage /= count;
		if (frameTimeAverage > 0)
		{
			m_FPS = 1000.0f / frameTimeAverage;
		}
		else
		{
			m_FPS = 60.0f;
		}
	}
}