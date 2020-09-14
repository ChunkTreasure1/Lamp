#include "lppch.h"
#include "AudioListenerComponent.h"

namespace Lamp
{
	bool AudioListenerComponent::s_Registered = LP_REGISTER_COMPONENT(AudioListenerComponent);

	void AudioListenerComponent::Initialize()
	{
	}

	void AudioListenerComponent::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<EntityPositionChangedEvent>(LP_BIND_EVENT_FN(AudioListenerComponent::OnPositionChanged));
	}

	void AudioListenerComponent::CalculateForwardAndUp()
	{
		m_Forward.x = glm::sin(glm::radians(m_pEntity->GetRotation().y)) * glm::cos(glm::radians(m_pEntity->GetRotation().x));
		m_Forward.y = glm::sin(glm::radians(-m_pEntity->GetRotation().x));
		m_Forward.z = glm::cos(glm::radians(m_pEntity->GetRotation().x)) * glm::cos(glm::radians(m_pEntity->GetRotation().y));
	
		m_Up = glm::cross(glm::cross(m_Forward, glm::vec3(0.f, 1.f, 0.f)), m_Forward);
	}

	bool AudioListenerComponent::OnPositionChanged(EntityPositionChangedEvent& e)
	{
		CalculateForwardAndUp();
		ListenerAttributes attr(m_pEntity->GetPosition(), glm::vec3(0.f), m_Forward, m_Up);

		AudioEngine::Get()->SetListenerAttributes(m_Listener, attr);

		return true;
	}
}