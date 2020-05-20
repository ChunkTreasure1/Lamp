#pragma once

namespace Lamp
{
	class FrameTime
	{
		public:
			void Begin();
			float End();

			inline const float GetFrameTime() const { return m_FrameTime; }
			inline const float GetFramesPerSecond() const { return m_FramesPerSecond; }

	private:
		void CalculateFramesPerSecond();

	private:
		float m_FramesPerSecond;
		float m_FrameTime;
		float m_StartTicks;
	};
}