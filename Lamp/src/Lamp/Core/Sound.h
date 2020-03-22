#pragma once

#include <soloud.h>

namespace Lamp
{
	class Sound
	{
	public:
		static void Initialize()
		{
			m_SoundBase.init();
		}

		static void UnInitialize()
		{
			m_SoundBase.deinit();
		}

		static SoLoud::Soloud& Get() { return m_SoundBase; }

	private:
		static SoLoud::Soloud m_SoundBase;

	};

	SoLoud::Soloud Sound::m_SoundBase;
}