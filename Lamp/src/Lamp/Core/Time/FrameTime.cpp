#include "lppch.h"
#include "FrameTime.h"

#include <GLFW/glfw3.h>

namespace Lamp
{
	void FrameTime::Begin()
	{
		m_StartTicks = (float)glfwGetTime();
	}

	float FrameTime::End()
	{
		CalculateFramesPerSecond();

		return m_FramesPerSecond;
	}

	void FrameTime::CalculateFramesPerSecond()
	{
		static const int numSamples = 100;
		static float frameTimes[numSamples];
		static int currentFrame = 0;
		static float prevTicks = (float)glfwGetTime();

		float currentTicks = (float)glfwGetTime();

		m_FrameTime = currentTicks - prevTicks;
		frameTimes[currentFrame % numSamples] = m_FrameTime;

		prevTicks = currentTicks;
		int count = 0;

		currentFrame++;
		if (currentFrame < numSamples)
		{
			count = currentFrame;
		}
		else
		{
			count = numSamples;
		}

		float frameTimeAverage = 0;
		for (size_t i = 0; i < count; i++)
		{
			frameTimeAverage += frameTimes[i];
		}
		frameTimeAverage /= count;

		if (frameTimeAverage > 0)
		{
			m_FramesPerSecond = 1.f / frameTimeAverage;
		}
		else 
		{
			m_FramesPerSecond = 60.f;
		}
	}
}