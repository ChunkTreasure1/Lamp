#include "lppch.h"
#include "AudioComponent.h"

#include "Lamp/Audio/AudioEngine.h"

namespace Lamp
{
	bool AudioComponent::s_Registered = LP_REGISTER_COMPONENT(AudioComponent);

	void AudioComponent::Initialize()
	{
		if (m_PlayOnStart)
		{
			Play();
		}
	}

	void AudioComponent::OnEvent(Event& e)
	{
	}

	void AudioComponent::Play()
	{
		if (m_EventName != "")
		{
			AudioEngine::Get()->PlayEvent(std::string("event:/") + m_EventName, m_pEntity->GetPosition());
		}
	}

	void AudioComponent::Stop()
	{
		if (m_EventName != "")
		{
			AudioEngine::Get()->StopEvent(m_EventName);
		}
	}

}