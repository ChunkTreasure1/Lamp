#pragma once

#include <Lamp/Rendering/Texture2D/Texture2D.h>
#include <Lamp/Core/Time/Timestep.h>

namespace Sandbox3D
{
	class AnimatedIcon
	{
	public:
		AnimatedIcon(uint32_t frameCount, const std::filesystem::path& firstFrame);

		void Animate(Lamp::Timestep ts);

		inline Ref<Lamp::Texture2D> GetCurrentFrame() { return m_CurrentTexture; }

	private:
		std::vector<Ref<Lamp::Texture2D>> m_Textures;
		Ref<Lamp::Texture2D> m_CurrentTexture;
	};
}