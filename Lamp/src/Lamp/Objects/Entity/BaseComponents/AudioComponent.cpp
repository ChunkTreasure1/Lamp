#include "lppch.h"
#include "AudioComponent.h"

#include "Lamp/Audio/AudioEngine.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(AudioComponent);

	AudioComponent::AudioComponent()
		: EntityComponent("AudioComponent"), m_EventName(""), m_PlayOnStart(false)
	{

	}

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

	void AudioComponent::SetComponentProperties()
	{
		m_componentProperties =
		{
			{ PropertyType::String, "Event name", RegisterData(&m_EventName) },
			{ PropertyType::Bool, "Play on start", RegisterData(&m_PlayOnStart) }
		};
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