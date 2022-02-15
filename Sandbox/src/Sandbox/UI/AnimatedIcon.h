#pragma once

#include <Lamp/Core/Time/Timestep.h>

#include <Lamp/Event/ApplicationEvent.h>
#include <Lamp/Rendering/Textures/Texture2D.h>

namespace Sandbox
{
	class AnimatedIcon
	{
	public:
		AnimatedIcon(const std::filesystem::path& firstFrame, uint32_t frameCount, float animTime = 1.f);

		void OnEvent(Lamp::Event& e);

		inline Ref<Lamp::Texture2D> GetCurrentFrame() { return m_CurrentTexture; }

		inline void Play() { m_Playing = true; }
		inline void Stop() { m_Playing = false; m_CurrentTexture = m_Textures[0]; }

	private:
		bool Animate(Lamp::AppUpdateEvent& e);

	private:
		std::vector<Ref<Lamp::Texture2D>> m_Textures;
		Ref<Lamp::Texture2D> m_CurrentTexture;

		float m_AnimTime;
		uint32_t m_FrameCount;
		float m_PerFrameTime;
		bool m_Playing = false;

		float m_CurrTime = 0.f;
		uint32_t m_CurrFrame = 0;
	};
}