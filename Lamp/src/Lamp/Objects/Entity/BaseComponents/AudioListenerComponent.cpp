#include "lppch.h"
#include "AudioListenerComponent.h"

namespace Lamp
{
	LP_REGISTER_COMPONENT(AudioListenerComponent);

	AudioListenerComponent::AudioListenerComponent()
		: EntityComponent("AudioListenerComponent"), m_forward(0.f), m_up(0.f), m_listener(0)
	{
	}

	void AudioListenerComponent::Initialize()
	{
	}

	void AudioListenerComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<ObjectPositionChangedEvent>(LP_BIND_EVENT_FN(AudioListenerComponent::OnPositionChanged));
	}

	void AudioListenerComponent::CalculateForwardAndUp()
	{
		m_forward.x = glm::sin(m_pEntity->GetRotation().y) * glm::cos(m_pEntity->GetRotation().x);
		m_forward.y = glm::sin(-m_pEntity->GetRotation().x);
		m_forward.z = glm::cos(m_pEntity->GetRotation().x) * glm::cos(m_pEntity->GetRotation().y);
	
		m_up = glm::cross(glm::cross(m_forward, glm::vec3(0.f, 1.f, 0.f)), m_forward);
	}

	bool AudioListenerComponent::OnPositionChanged(ObjectPositionChangedEvent& e)
	{
		CalculateForwardAndUp();
		ListenerAttributes attr(m_pEntity->GetPosition(), glm::vec3(0.f), m_forward, m_up);

		AudioEngine::Get()->SetListenerAttributes(m_listener, attr);

		return true;
	}
}