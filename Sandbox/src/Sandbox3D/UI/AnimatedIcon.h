#pragma once

#include <Lamp/Rendering/Texture2D/Texture2D.h>
#include <Lamp/Core/Time/Timestep.h>

#include <Lamp/Event/ApplicationEvent.h>

namespace Sandbox3D
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
		float m_PerFrameTime;
		uint32_t m_FrameCount;
		bool m_Playing = false;

		float m_CurrTime = 0.f;
		uint32_t m_CurrFrame = 0;
	};
}